@echo off
set validator="C:/VulkanSDK/1.1.82.1/Bin32/glslangValidator.exe"

%validator% -V main.vert -o main.vert.spv
%validator% -V main.frag -o main.frag.spv

@echo on

pause