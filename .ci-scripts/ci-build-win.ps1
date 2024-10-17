.ci-scripts\vsdevenv.ps1 64

function Invoke-NativeCommand() {
    if ($args.Count -eq 0) {
        throw "Must supply some arguments."
    }

    $command = $args[0]
    $commandArgs = @()
    if ($args.Count -gt 1) {
        $commandArgs = $args[1..($args.Count - 1)]
    }

    & $command $commandArgs
    $result = $LASTEXITCODE

    if ($result -ne 0) {
        throw "$command $commandArgs exited with code $result."
    }
}


$rizin_path = "C:$env:HOMEPATH\AppData\Local\Programs\rizin"
$env:PATH = "$env:PATH;C:$env:HOMEPATH\AppData\Local\Programs\rizin\bin"
$env:PKG_CONFIG_PATH = "C:$env:HOMEPATH\AppData\Local\Programs\rizin\lib\pkgconfig"
$env:CFLAGS = "-IC:$env:HOMEPATH\AppData\Local\Programs\rizin\include\librz -IC:$env:HOMEPATH\AppData\Local\Programs\rizin\include\librz\sdb"
$env:LDFLAGS = "-LC:$env:HOMEPATH\AppData\Local\Programs\rizin\lib"


Invoke-NativeCommand meson setup --buildtype=release --prefix="$rizin_path" build
Invoke-NativeCommand ninja -C build install
rizin.exe -e log.level=2 -qc "Lc"
rizin.exe -qc "af ; pdd" "C:\Windows\System32\calc.exe"
