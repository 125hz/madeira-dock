# ml1820: native Windows test using the installed client's cached sign-in.
# Account identifiers are not printed; credentials remain inside Valve code.
param([Parameter(Mandatory=$true)][uint32]$AppId, [switch]$Launch, [string]$LibraryRoot)
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
New-Item -ItemType Directory -Path (Join-Path $repo '.build/logs') -Force | Out-Null
$steamPath = (Get-ItemProperty 'HKCU:/Software/Valve/Steam').SteamPath
$accountFile = Get-Content (Join-Path $steamPath 'config/loginusers.vdf') -Raw
$accounts = @([regex]::Matches($accountFile, '"(?<id>\d{17})"\s*\{(?<body>[^}]+)\}'))
if ($accounts.Count -eq 1) {
    $selected = $accounts[0]
} else {
    $recent = @($accounts | Where-Object { $_.Groups['body'].Value -match '"MostRecent"\s*"1"' })
    if ($recent.Count -ne 1) { throw 'No unambiguous cached account; account selection is required.' }
    $selected = $recent[0]
}
$accountName = [regex]::Match($selected.Groups['body'].Value, '"AccountName"\s*"([^"]+)"').Groups[1].Value
if (!$accountName) { throw 'Cached account metadata has no account name.' }
$settings = @{
    MADEIRA_STEAM_HOST_ACCOUNT = $accountName
    MADEIRA_STEAM_HOST_STEAMID = $selected.Groups['id'].Value
    MADEIRA_STEAM_HOST_APPID = "$AppId"
    MADEIRA_STEAM_HOST_PROBE = '1'
    MADEIRA_STEAM_HOST_SESSION = '1'
    MADEIRA_STEAM_HOST_LOGIN = '1'
    MADEIRA_STEAM_HOST_LOG = (Join-Path $repo '.build/logs/session.txt')
    MADEIRA_STEAM_HOST_LAUNCH = '0'
}
if ($Launch) {
    if (!$LibraryRoot) { throw 'Launch requires an explicit LibraryRoot.' }
    $rootPath = (Resolve-Path -LiteralPath $LibraryRoot).Path
    $manifest = Get-Content -LiteralPath (Join-Path $rootPath "steamapps/appmanifest_$AppId.acf") -Raw
    $installName = [regex]::Match($manifest, '"installdir"\s*"([^"]+)"').Groups[1].Value
    if (!$installName -or $installName -match '[/\\:]' -or $installName -in '.', '..') {
        throw 'Invalid install directory in app manifest.'
    }
    $settings.MADEIRA_STEAM_HOST_EXPECTED_INSTALL = (Resolve-Path -LiteralPath (Join-Path $rootPath "steamapps/common/$installName")).Path
    $settings.MADEIRA_STEAM_HOST_LAUNCH = '1'
}
$previous = @{}
try {
    foreach ($key in $settings.Keys) {
        $previous[$key] = [Environment]::GetEnvironmentVariable($key, 'Process')
        [Environment]::SetEnvironmentVariable($key, $settings[$key], 'Process')
    }
    & (Join-Path $repo '.build/windows/dockhost-x86_64.exe')
    $result = $LASTEXITCODE
} finally {
    foreach ($key in $previous.Keys) {
        [Environment]::SetEnvironmentVariable($key, $previous[$key], 'Process')
    }
}
exit $result
