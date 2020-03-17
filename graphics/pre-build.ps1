if (!(Test-Path "./shaders/compiled")) {
  New-Item -ItemType "directory" -Path "./shaders/" -Name "compiled" | Out-Null
}

foreach ($file_name in Get-ChildItem "./shaders/compiled/*.*" -Name) {
  $delete_file_path = "./shaders/compiled/${file_name}"

  Write-Host "Deleting '${delete_file_path}'."

  Remove-Item -Path $delete_file_path
}

foreach ($file_name in Get-ChildItem "./shaders/*.*" -Name) {
  $input_file_path = "./shaders/${file_name}"
  
  $file_name_without_extension = [io.path]::GetFileNameWithoutExtension($file_name)
  $output_file_path = "./shaders/compiled/${file_name_without_extension}.spirv"
  
  Write-Host "Compiling '${input_file_path}' into '${output_file_path}'."

  $complie_command = "glslc ${input_file_path} -o ${output_file_path}"
  Invoke-Expression($complie_command)
}