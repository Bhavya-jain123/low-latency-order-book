param([string]$BuildDirectory = "build-release", [string]$OutputDirectory = "benchmark-results")

New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
foreach ($count in 10000, 100000, 1000000) {
    foreach ($pattern in "noncrossing", "crossing", "sameprice", "mixed", "burst") {
        & "$BuildDirectory/order_engine_bench.exe" --count $count --seed 42 --pattern $pattern --cancel-probability 0.10 --csv "$OutputDirectory/$count-$pattern.csv"
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }
}
