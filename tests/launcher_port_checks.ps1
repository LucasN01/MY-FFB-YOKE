$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
# Load only function definitions: never run the launcher or touch physical serial ports.
$source = Join-Path (Split-Path $PSScriptRoot) 'Iniciar-Simulador.ps1'
$tokens = $null; $parseErrors = $null
$ast = [Management.Automation.Language.Parser]::ParseFile($source, [ref]$tokens, [ref]$parseErrors)
if ($parseErrors.Count) { throw ($parseErrors | Out-String) }
foreach ($name in @('Get-YokeCandidates','Find-YokePort','Test-MobiIgnoresPort','Set-MobiValue')) {
    $definition = $ast.Find({ param($node) $node -is [Management.Automation.Language.FunctionDefinitionAst] -and $node.Name -eq $name }, $true)
    if (!$definition) { throw "Missing function $name" }
    . ([scriptblock]::Create($definition.Extent.Text))
}
$checks = 0
function Assert($Condition, [string]$Message) {
    $script:checks++
    if (!$Condition) { throw "FAIL: $Message" }
}
function Expect-Error([scriptblock]$Action, [string]$Text) {
    $caught = ''
    try { & $Action | Out-Null } catch { $caught = $_.Exception.Message }
    Assert ($caught -like "*$Text*") "Expected error '$Text', received '$caught'"
}
$devices = @(
    [pscustomobject]@{ FriendlyName='Pico (COM50)'; InstanceId='USB\VID_2E8A&PID_000F&MI_00\pico' },
    [pscustomobject]@{ FriendlyName='Arduino (COM3)'; InstanceId='USB\VID_2341&PID_0043\arduino' },
    [pscustomobject]@{ FriendlyName='CH340 (COM7)'; InstanceId='USB\VID_1A86&PID_7523\generic' },
    [pscustomobject]@{ FriendlyName='Stale Pico (COM13)'; InstanceId='USB\VID_2E8A&PID_000F\old' },
    [pscustomobject]@{ FriendlyName='Wrong PID (COM8)'; InstanceId='USB\VID_2E8A&PID_000A\other' },
    [pscustomobject]@{ FriendlyName='Prefix mismatch (COM9)'; InstanceId='USB\VID_2E8A&PID_000FF\other' }
)
$selected = @(Get-YokeCandidates -Devices $devices -PresentPorts @('COM50','COM3','COM7','COM8','COM9'))
Assert ($selected.Count -eq 1 -and $selected[0].Port -eq 'COM50') 'Only present Pico 2 selected'
$devices[0].FriendlyName = 'Dispositivo serie USB (com12)'
$selected = @(Get-YokeCandidates -Devices $devices -PresentPorts @('COM12','COM3','COM7'))
Assert ($selected.Count -eq 1 -and $selected[0].Port -eq 'COM12') 'Generic/localized name and changed COM work'
Assert (@(Get-YokeCandidates -Devices @() -PresentPorts @()).Count -eq 0) 'Empty inventory'
Assert (@(Get-YokeCandidates -Devices $devices -PresentPorts @('COM3','COM7')).Count -eq 0) 'Other boards never selected'

# Discovery policy with simulated USB candidates and simulated firmware replies.
function Get-YokeCandidates { return $script:candidates }
function Get-YokeFirmware([string]$PortName) {
    $script:queried += $PortName
    if ($script:replies[$PortName] -eq 'BUSY') { throw 'Puerto ocupado' }
    return $script:replies[$PortName]
}
$script:candidates = @(); $script:queried = @(); $script:replies = @{}
Expect-Error { Find-YokePort 'YOKE17SC7' } 'No se encontro una Pico 2'
Assert ($script:queried.Count -eq 0) 'No fallback scan of unrelated serial ports'
foreach ($port in @('COM12','COM50','COM103')) {
    $script:candidates = @([pscustomobject]@{ Port=$port; InstanceId='USB\pico' })
    $script:replies = @{}; $script:replies[$port]='YOKE17SC7'
    $result = Find-YokePort 'YOKE17SC7'
    Assert ($result.Port -eq $port -and $result.Firmware -eq 'YOKE17SC7') "Automatic discovery on $port"
}
$script:replies['COM103']='YOKE17SC6'
Expect-Error { Find-YokePort 'YOKE17SC7' } 'responde YOKE17SC6'
$script:replies['COM103']='BUSY'
Expect-Error { Find-YokePort 'YOKE17SC7' } 'Puerto ocupado'
$script:candidates = @(
    [pscustomobject]@{ Port='COM12'; InstanceId='USB\pico1' },
    [pscustomobject]@{ Port='COM50'; InstanceId='USB\pico2' }
)
$script:replies = @{ COM12='YOKE17SC7'; COM50='YOKE17SC7' }
Expect-Error { Find-YokePort 'YOKE17SC7' } 'mas de un yoke'
$script:replies['COM50']='YOKE17SC5'
Assert ((Find-YokePort 'YOKE17SC7').Port -eq 'COM12') 'Use verified firmware, not first arbitrary board'
Assert ((Find-YokePort 'YOKE17SC5').Port -eq 'COM50') 'Legacy mode requires its own firmware'

[xml]$mobi = '<configuration><userSettings><MobiFlight.Properties.Settings><setting name="IgnoreComPorts"><value>True</value></setting><setting name="IgnoredComPortsList"><value>COM7, COM50</value></setting></MobiFlight.Properties.Settings></userSettings></configuration>'
Assert (Test-MobiIgnoresPort $mobi 'COM50') 'Detected port already reserved'
Assert (!(Test-MobiIgnoresPort $mobi 'COM5')) 'Exact COM comparison'
Set-MobiValue $mobi 'IgnoreComPorts' 'False'
Assert (!(Test-MobiIgnoresPort $mobi 'COM50')) 'Disabled exclusion rejected'
Set-MobiValue $mobi 'IgnoreComPorts' 'True'
Set-MobiValue $mobi 'IgnoredComPortsList' 'COM7,COM50,COM103'
Assert (Test-MobiIgnoresPort $mobi 'COM103') 'New detected port reserved'
Assert (Test-MobiIgnoresPort $mobi 'COM7') 'Other exclusions preserved'
Write-Output "PASS: $checks launcher discovery checks. No hardware ports opened."
