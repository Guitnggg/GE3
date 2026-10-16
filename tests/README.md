# Engine tests

`EngineTests` is a small, dependency-free console test project. It exercises CPU-only engine code and does not create a window or DirectX device.

Build and run from the repository root:

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe" EngineTests.vcxproj /p:Configuration=Debug /p:Platform=x64
& ".\x64\Debug\EngineTests.exe"
```

Add a test by including `tests/TestFramework.h` and declaring a `TEST_CASE`. A failed requirement throws an exception; the runner reports the test name and continues with the remaining cases. Keep tests independent and avoid GPU, window, or global engine initialization unless a dedicated integration-test project is introduced.

