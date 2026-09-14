# OpenGL 실습 솔루션

모든 실습이 `Config`의 CMake 설정과 GLFW, GLEW, GLM 설치를 함께 사용합니다.

```text
OpenGL
├─ OpenGL.slnx
├─ Config
│  ├─ CMakeLists.txt
│  ├─ CMakePresets.json
│  ├─ vcpkg.json
│  ├─ .gitignore
│  └─ README.md
├─ out
├─ Project1
│  ├─ Project1-1.cpp
│  ├─ Project1-2.cpp
│  ├─ Project1-3.cpp
│  └─ Project1-4.cpp
└─ Project2
   ├─ Project2-1.cpp
   └─ Project2-2.cpp
```

## Visual Studio에서 실행

1. `OpenGL.slnx`를 엽니다.
2. `Project1-1`, `Project1-2`, `Project1-3` 중 하나를 시작 프로젝트로 지정합니다.
3. `F5` 또는 `Ctrl+F5`를 누릅니다.

## 명령줄에서 구성 및 빌드

Visual Studio의 Developer PowerShell에서 `OpenGL/Config` 폴더로 이동한 후 실행합니다.

```powershell
cmake --preset windows-x64
cmake --build --preset debug
```

## Project2-1 추가

1. `Project2/Project2-1.cpp`를 만듭니다. Visual Studio의 **새 프로젝트**는 선택하지 않습니다.
2. `Config/CMakeLists.txt` 마지막에 다음 줄을 추가합니다.

```cmake
add_opengl_example(Project2-1 ../Project2/Project2-1.cpp Project2)
```

3. `Config` 폴더에서 `cmake --preset windows-x64`를 실행합니다.
4. `OpenGL.slnx`에서 **추가 > 기존 프로젝트**를 선택합니다.
5. 반드시 CMake가 생성한 `out/build/windows-x64/Project2-1.vcxproj`를 추가합니다.

`Project1`, `Project2` 폴더에는 CPP 소스만 두며, Visual Studio가 별도로 생성한 `.vcxproj`는 사용하지 않습니다.

빌드 결과와 공용 라이브러리는 모두 루트 `out`에 저장됩니다.
