// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "RacketPointScoring",
    products: [
        // dynamic so that dlsym(RTLD_DEFAULT) can find rp_* symbols at runtime
        .library(
            name: "RacketPointScoring",
            type: .dynamic,
            targets: ["RacketPointScoring"]
        )
    ],
    targets: [
        .target(
            name: "RacketPointScoring",
            path: ".",
            exclude: ["tests", "CMakeLists.txt"],
            sources: ["padel_scoring.c", "rp_ffi.c"],
            publicHeadersPath: "."
        )
    ]
)
