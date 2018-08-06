del /f /s /q .\fs\*.bak
for /f "tokens=*" %%f in ('dir /b /ad /s .\^|sort') do rd "%%f"
makefsdata.exe -e