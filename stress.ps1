# stress.ps1: compare ob.exe and brute.exe on random inputs from gen.exe
g++ -std=c++17 -O2 -Wall main.cpp -o ob;    if (-not $?) { "main.cpp failed to compile"; exit 1 }
g++ -std=c++17 -O2 -Wall brute.cpp -o brute; if (-not $?) { "brute.cpp failed to compile"; exit 1 }
g++ -std=c++17 -O2 -Wall gen.cpp -o gen;     if (-not $?) { "gen.cpp failed to compile"; exit 1 }

$runs = 1000
for ($seed = 1; $seed -le $runs; $seed++) {
    $inp = @(.\gen.exe $seed)
    $a   = @($inp | .\ob.exe)
    $b   = @($inp | .\brute.exe)
    if (Compare-Object $a $b -SyncWindow 0) {
        $inp | Set-Content fail_input.txt -Encoding ascii
        "MISMATCH at seed $seed (input saved to fail_input.txt)"
        "--- ob.exe ---";    $a
        "--- brute.exe ---"; $b
        exit 1
    }
    if ($seed % 100 -eq 0) { "$seed / $runs passed" }
}
"All $runs random tests passed"