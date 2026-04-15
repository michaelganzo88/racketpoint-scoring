// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "RacketPointScoring",
    products: [
        .library(
            name: "RacketPointScoring",
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
