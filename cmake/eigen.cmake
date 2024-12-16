FetchContent_Declare(
    eigen
    GIT_REPOSITORY  https://gitlab.com/libeigen/eigen.git
    GIT_TAG         3.4
)

set(EIGEN_BUILD_TESTING OFF)
set(EIGEN_MPL2_ONLY ON)
set(EIGEN_BUILD_PKGCONFIG OFF)
set(EIGEN_BUILD_DOC OFF)

FetchContent_MakeAvailable(eigen)

# After FetchContent_MakeAvailable, you can get the source directory:
FetchContent_GetProperties(eigen)
set(EIGEN_INCLUDE_DIR ${eigen_SOURCE_DIR})