rmdir /s /q ipch
rmdir /s /q Debug
rmdir /s /q .vs
rmdir /s /q obj
rmdir /s /q ../obj

del *.user
del *.exe

del /q ..\lib\vs2017\*.lib
del /q ..\lib\vs2017\*.pdb
