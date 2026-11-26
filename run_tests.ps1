# run_tests.ps1: compile, run every tests\*.txt, compare with its .expected file
g++ -std=c++17 -O2 -Wall main.cpp -o ob
if (-not $?) { Write-Output "Compile failed"; exit 1 }
$fail = 0
foreach ($in in Get-ChildItem tests\*.txt) {
    $expFile = [IO.Path]::ChangeExtension($in.FullName, ".expected")
    if (-not (Test-Path $expFile)) { Write-Output "MISSING $($in.BaseName).expected"; $fail++; continue }
    $actual   = @(Get-Content $in.FullName | .\ob.exe)
    $expected = @(Get-Content $expFile)
    if (Compare-Object $actual $expected -SyncWindow 0) { Write-Output "FAIL $($in.Name)"; $fail++ }
    else { Write-Output "PASS $($in.Name)" }
}
if ($fail -eq 0) { Write-Output "All tests passed" } else { Write-Output "$fail test(s) failed" }
