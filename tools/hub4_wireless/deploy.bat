echo Run this batch file in the directory where the application executable lives.
echo All files needed for deployment are copied to the dist directory within
echo this directory.
echo Note that the Qt bin dir must be in your PATH. Inserting the dir before
echo windeployqt.exe will not fix this.

windeployqt.exe --release --force --dir dist --qmldir ..\..\hub4_wireless\qml hub4_wireless.exe

rem windeployqt.exe copies everything, except the application binary...
copy hub4_wireless.exe dist
