#ifndef IRGLAB_TRACKED_POINTER_HPP
#define IRGLAB_TRACKED_POINTER_HPP


#include "external/pch.hpp"

#include "standard/type_traits.hpp"


namespace il
{
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedTemplateParameterInspection"

    // Having requirements for the tracker type results in errors because if the tracker type depends
    // on tracked_pointer, which is what the tracked_pointer is all about, during the instantiation of a tracker,
    // the tracker tries to instantiate a tracked_pointer, which in turn tries to instantiate a tracker to check
    // requirements and so on... Tracked_pointer introduces cyclic dependencies and therefore, it should be used very
    // carefully. Thankfully, the tracked_pointer works with incomplete types, so it is not entirely dependent on
    // the tracker type. The same is true for standard pointer types as well if you are creating a tree node type.
    template<typename InnerType, typename TrackerType>
    [[maybe_unused]] constexpr bool are_tracked_pointer_types_supported = true;

#pragma clang diagnostic pop


    template<typename InnerType, typename TrackerType>
    struct [[maybe_unused]] tracked_pointer
    {
        using inner_type [[maybe_unused]] = InnerType;
        using tracker_type [[maybe_unused]] = TrackerType;


        template<
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedLocalVariable"
                typename Dummy = void, std::enable_if_t<
                        std::is_same_v<Dummy, void> &&
                        are_tracked_pointer_types_supported<inner_type, tracker_type>, int> = 0>
#pragma clang diagnostic pop
        [[nodiscard, maybe_unused]] explicit tracked_pointer(std::shared_ptr<InnerType> inner) :
                _inner{std::move(inner)},
                _trackers{std::make_shared<std::vector<std::weak_ptr<TrackerType>>>(0)}
        { }

        [[maybe_unused]] void prune() const
        {
            _trackers->erase(
                    std::remove_if(
                            _trackers->begin(), _trackers->end(),
                            [](const std::weak_ptr<TrackerType>& tracker) -> bool
                            {
                                return tracker.expired();
                            }), _trackers->end());
        }


        [[nodiscard, maybe_unused]] InnerType& operator*() const noexcept
        {
            return *_inner;
        }

        [[nodiscard, maybe_unused]] InnerType* operator->() const noexcept
        {
            return _inner.get();
        }


        [[nodiscard, maybe_unused]] std::shared_ptr<InnerType> inner() const noexcept
        {
            return _inner;
        }

        [[nodiscard, maybe_unused]] std::shared_ptr<const std::vector<std::weak_ptr<TrackerType>>> trackers() const
        {
            return std::const_pointer_cast<const std::vector<std::weak_ptr<TrackerType>>>(_trackers);
        }


        [[nodiscard, maybe_unused]] bool operator==(const tracked_pointer& other) const
        {
            return this->_inner == other._inner;
        }


        [[maybe_unused]] void operator+=(const std::shared_ptr<TrackerType>& owner) const
        {
            _trackers->emplace_back(std::weak_ptr<TrackerType>(owner));
        }

        [[maybe_unused]] void operator+=(const tracked_pointer& other) const
        {
            if (*this == other && this->_trackers != other._trackers)
            {
                other._trackers->erase(
                        std::remove_if(
                                other._trackers->begin(), other._trackers->end(),
                                [this](const std::weak_ptr<TrackerType>& tracker) -> bool
                                {
                                    if (tracker.expired()) return true;

                                    const auto tracker_lock = tracker.lock();
                                    return std::find_if(
                                            this->_trackers->begin(), this->_trackers->end(),
                                            [&tracker_lock](const std::weak_ptr<TrackerType>& this_tracker) -> bool
                                            {
                                                return !this_tracker.expired() and
                                                       this_tracker.lock() == tracker_lock;
                                            }) != this->_trackers->end();
                                }), other._trackers->end());

                this->_trackers->insert(
                        this->_trackers->end(),
                        other._trackers->begin(), other._trackers->end());

                other._trackers = this->_trackers;
            }
        }

        [[nodiscard, maybe_unused]] bool operator&&(const std::shared_ptr<TrackerType> tracker) const
        {
            return std::find_if(
                    _trackers->begin(), _trackers->end(),
                    [&tracker](const std::weak_ptr<TrackerType>& this_tracker)
                    {
                        return !this_tracker.expired() and this_tracker.lock() == tracker;
                    }) != _trackers->end();
        }


        template<typename ConversionInnerType, typename ConversionTrackerType>
        friend
        struct tracked_pointer;

    private:
        explicit tracked_pointer(
                std::shared_ptr<InnerType> shared,
                std::shared_ptr<std::vector<std::weak_ptr<TrackerType>>> trackers) :
                _inner{std::move(shared)},
                _trackers{std::move(trackers)}
        { }

    public:
        template<typename ConversionInnerType, typename ConversionTrackerType>
        [[maybe_unused]] static constexpr bool are_conversion_types_allowed =
                are_tracked_pointer_types_supported<ConversionInnerType, ConversionTrackerType> &&
                is_one_of<
                        ConversionInnerType, std::conditional_t<
                                std::is_const_v<InnerType>,
                                const InnerType, std::variant<const InnerType, InnerType>>>;

        template<
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedLocalVariable"
                typename ConversionInnerType, typename ConversionTrackerType, std::enable_if_t<
                        are_conversion_types_allowed<ConversionInnerType, ConversionTrackerType>, int> = 0>
#pragma clang diagnostic pop
        [[nodiscard, maybe_unused]] explicit operator tracked_pointer<ConversionInnerType, ConversionTrackerType>()
        {
            if constexpr(std::is_same_v<TrackerType, ConversionTrackerType>)
                return tracked_pointer<ConversionInnerType, ConversionTrackerType>
                        {
                                std::const_pointer_cast<ConversionInnerType>(_inner),
                                _trackers
                        };

            else
                return tracked_pointer<ConversionInnerType, ConversionTrackerType>
                        {
                                std::const_pointer_cast<ConversionInnerType>(_inner)
                        };
        }

        friend std::hash<tracked_pointer>;

    private:
        std::shared_ptr<InnerType> _inner;
        std::shared_ptr<std::vector<std::weak_ptr<TrackerType>>> _trackers;
    };
}


template<typename InnerType, typename TrackerType>
struct [[maybe_unused]] std::hash<il::tracked_pointer<InnerType, TrackerType>>
{
private:
    static inline const std::hash<std::shared_ptr<InnerType>> _inner_hasher{ };

public:
    [[maybe_unused]] size_t operator()(const il::tracked_pointer<InnerType, TrackerType>& key) const noexcept
    {
        return _inner_hasher(key._inner);
    }
};


#endif //IRGLAB_TRACKED_POINTER_HPP
