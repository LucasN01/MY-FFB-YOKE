[CmdletBinding()]
param([switch]$Comprobar, [switch]$SoloPreparar, [switch]$DetectarPlaca, [switch]$Realismo = $true,
      [ValidateSet('3','4')][string]$Version = '4')
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

# Rutas de esta PC. Escenario vacio = usar el predeterminado de Prepar3D.
$PreparBase = if ($Version -eq '3') { ${env:ProgramFiles(x86)} } else { $env:ProgramFiles }
$PreparExe = Join-Path $PreparBase "Lockheed Martin\Prepar3D v$Version\Prepar3D.exe"
$MobiExe = Join-Path $env:LOCALAPPDATA 'MobiFlight\MobiFlight Connector\MFConnector.exe'
$MobiProyecto = 'C:\Users\norie\OneDrive\Documentos\prepar lucas.mfproj'
$Escenario = ''
$Puerto = $null # Se resuelve por USB + identificacion del firmware, nunca por numero fijo.
$EsperaMaximaSegundos = 1800
$PreparCfg = Join-Path $env:APPDATA "Lockheed Martin\Prepar3D v$Version\Prepar3D.cfg"
$BridgeDir = Join-Path $PSScriptRoot 'SimConnectBridge\bin\Win32\Release'
$WaitExe = Join-Path $BridgeDir 'WaitForFlight.exe'
if ($Realismo) { $BridgeDir = Join-Path $PSScriptRoot 'SimConnectBridge\bin\Win32\RealismoSC8' }
$BridgeExe = Join-Path $BridgeDir 'YokeSimConnectBridge.exe'
$FirmwareEsperado = if ($Realismo) { 'YOKE17SC8' } else { 'YOKE17SC5' }
$mutex = New-Object System.Threading.Mutex($false, 'Local\MiYoke_Prepar3D_Launcher')
$locked = $false
$transcript = $false

function Assert-File([string]$Path) {
    if (!(Test-Path -LiteralPath $Path -PathType Leaf)) { throw "No se encontro: $Path" }
}
function Backup-Once([string]$Path) {
    $backup = "$Path.antes-del-lanzador.bak"
    if (!(Test-Path -LiteralPath $backup)) { Copy-Item -LiteralPath $Path -Destination $backup }
}
function Set-MobiValue($Document, [string]$Name, [string]$Value) {
    $parent = $Document.SelectSingleNode('/configuration/userSettings/MobiFlight.Properties.Settings')
    if (!$parent) { throw 'Formato de configuracion MobiFlight no reconocido.' }
    $node = $parent.SelectSingleNode("setting[@name='$Name']/value")
    if (!$node) {
        $setting = $Document.CreateElement('setting')
        $setting.SetAttribute('name', $Name)
        $setting.SetAttribute('serializeAs', 'String')
        $node = $Document.CreateElement('value')
        [void]$setting.AppendChild($node)
        [void]$parent.AppendChild($setting)
    }
    $node.InnerText = $Value
}

function Get-YokeCandidates {
    param(
        [object[]]$Devices = @(Get-PnpDevice -PresentOnly -ErrorAction Stop | Where-Object { $_.Class -eq 'Ports' }),
        [string[]]$PresentPorts = @([IO.Ports.SerialPort]::GetPortNames())
    )
    # Pico 2 ARM/TinyUSB de este proyecto: VID Raspberry Pi 2E8A, PID 000F.
    # Filtrar ANTES de abrir puertos: no sondear Arduino, CH340 ni placas genericas.
    foreach ($device in $Devices) {
        if ($device.InstanceId -notmatch '^USB\\VID_2E8A&PID_000F(?:&|\\)') { continue }
        $portMatch = [regex]::Match([string]$device.FriendlyName, '\((COM[0-9]+)\)', 'IgnoreCase')
        if (!$portMatch.Success) { continue }
        $portName = $portMatch.Groups[1].Value.ToUpperInvariant()
        if ($presentPorts -notcontains $portName) { continue }
        [pscustomobject]@{ Port = $portName; InstanceId = $device.InstanceId }
    }
}

function Get-YokeFirmware([string]$PortName) {
    # Solo identificacion ASCII. No enviar tramas armadas ni comandos de motor.
    # 115200 evita el reset a bootloader asociado a abrir/cerrar a 1200 baudios.
    $serial = New-Object System.IO.Ports.SerialPort
    $serial.PortName = $PortName
    $serial.BaudRate = 115200
    $serial.DtrEnable = $true
    $serial.ReadTimeout = 200
    $serial.WriteTimeout = 200
    $timer = [Diagnostics.Stopwatch]::StartNew()
    try {
        $serial.Open()
        $serial.DiscardInBuffer()
        $response = ''
        $nextQuery = 0
        while ($timer.ElapsedMilliseconds -lt 3000) {
            if ($timer.ElapsedMilliseconds -ge $nextQuery) {
                $serial.Write('?')
                $nextQuery = $timer.ElapsedMilliseconds + 200
            }
            $response += $serial.ReadExisting()
            $identity = [regex]::Match($response, '(?m)^YOKE17SC[0-9]+\r?\n')
            if ($identity.Success) { return $identity.Value.Trim() }
            if ($response.Length -gt 2048) { $response = $response.Substring($response.Length - 256) }
            Start-Sleep -Milliseconds 50
        }
        throw 'No respondio a la consulta de identificacion en 3 segundos.'
    } catch {
        throw "No se pudo identificar $PortName. $($_.Exception.Message) Cerra el monitor serie u otro programa que use la Pico y reintenta."
    } finally {
        $timer.Stop()
        $serial.Dispose()
    }
}

function Find-YokePort([string]$ExpectedFirmware) {
    $candidates = @(Get-YokeCandidates | Sort-Object Port -Unique)
    if (!$candidates.Count) {
        throw 'No se encontro una Pico 2 con USB VID_2E8A/PID_000F y puerto COM activo. Conectala con cable de datos y firmware del yoke; en modo BOOTSEL no tiene este puerto serie. No se consultaron otras placas.'
    }
    $confirmed = @()
    $details = @()
    foreach ($candidate in $candidates) {
        try {
            $firmware = Get-YokeFirmware $candidate.Port
            if ($firmware -eq $ExpectedFirmware) {
                $confirmed += [pscustomobject]@{
                    Port = $candidate.Port; InstanceId = $candidate.InstanceId; Firmware = $firmware
                }
            } else { $details += "$($candidate.Port) responde $firmware; se necesita $ExpectedFirmware." }
        } catch { $details += $_.Exception.Message }
    }
    if ($confirmed.Count -gt 1) {
        throw "Hay mas de un yoke compatible ($($confirmed.Port -join ', ')). Deja conectada una sola Pico del yoke para evitar elegir la incorrecta."
    }
    if (!$confirmed.Count) { throw "Pico 2 encontrada, pero no se confirmo el firmware del yoke. $($details -join ' ')" }
    return $confirmed[0]
}

function Test-MobiIgnoresPort($Document, [string]$PortName) {
    $enabled = $Document.SelectSingleNode("//setting[@name='IgnoreComPorts']/value")
    $ignored = $Document.SelectSingleNode("//setting[@name='IgnoredComPortsList']/value")
    if (!$enabled -or !$ignored -or $enabled.InnerText.Trim() -ine 'True') { return $false }
    $names = @($ignored.InnerText.Split(',') | ForEach-Object { $_.Trim() })
    return $names -contains $PortName
}

try {
    try { $locked = $mutex.WaitOne(0) } catch [System.Threading.AbandonedMutexException] { $locked = $true }
    if (!$locked) { Write-Host 'El lanzador ya esta en ejecucion.'; exit 0 }
    # No abrir la Pico ni cambiar DTR si el puente ya la esta usando.
    if (Get-Process -Name YokeSimConnectBridge -ErrorAction SilentlyContinue) {
        Write-Host 'El puente ya esta abierto. No se consultara su puerto ni se iniciara otro.'
        exit 0
    }
    if ($DetectarPlaca) {
        $yoke = Find-YokePort $FirmwareEsperado
        Write-Host "Yoke detectado: $($yoke.Port) | Firmware: $($yoke.Firmware)"
        Write-Host "USB: $($yoke.InstanceId)"
        Write-Host 'Solo identificacion. No se enviaron fuerzas ni se abrieron programas ni se modificaron configuraciones.'
        exit 0
    }
    if (!$Comprobar) {
        $logs = Join-Path $PSScriptRoot 'logs'
        [void][IO.Directory]::CreateDirectory($logs)
        Start-Transcript -Path (Join-Path $logs ('inicio-v' + $Version + '-' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.log')) | Out-Null
        $transcript = $true
        Write-Host "Lanzador: $PSCommandPath | Usuario: $([Environment]::UserName)"
    }
    foreach ($file in @($PreparExe, $MobiExe, $MobiProyecto, $PreparCfg, $BridgeExe, $WaitExe)) { Assert-File $file }
    # SimConnect local no selecciona una version por PID: impedir sesiones ambiguas.
    $simProcesses = @(Get-Process -Name Prepar3D -ErrorAction SilentlyContinue)
    foreach ($process in $simProcesses) {
        if (!$process.Path -or $process.Path -ine $PreparExe) {
            throw "Hay otra version de Prepar3D abierta (PID $($process.Id)). Cerrala antes de iniciar v$Version."
        }
    }
    if ($simProcesses.Count -gt 1) { throw 'Hay varias instancias de Prepar3D abiertas. Deja solo una.' }
    $prepar = $simProcesses | Select-Object -First 1
    if ($Version -eq '3') {
        $dllConfig = Join-Path $env:APPDATA 'Lockheed Martin\Prepar3D v3\dll.xml'
        Assert-File $dllConfig
        $dllXml = New-Object System.Xml.XmlDocument
        $dllXml.Load($dllConfig)
        $fsuipcNode = $dllXml.SelectSingleNode('//Launch.Addon[Name="FSUIPC 4"]/Path')
        if (!$fsuipcNode) { throw "Falta el registro FSUIPC 4 en $dllConfig" }
        $fsuipcPath = [Environment]::ExpandEnvironmentVariables($fsuipcNode.InnerText.Trim())
        if (![IO.Path]::IsPathRooted($fsuipcPath)) { $fsuipcPath = Join-Path (Split-Path $PreparExe) $fsuipcPath }
        Assert-File $fsuipcPath
        Assert-File (Join-Path (Split-Path $fsuipcPath) 'SimConnectP3D3.dll')
        Write-Host "FSUIPC V3: $fsuipcPath"
    }
    $cfgText = [IO.File]::ReadAllText($PreparCfg, [Text.Encoding]::Default)
    if (!$Escenario) {
        $match = [regex]::Match($cfgText, '(?im)^SITUATION=(.+)$')
        if (!$match.Success) { throw 'Prepar3D no tiene un escenario predeterminado. Guarda uno en tierra como predeterminado.' }
        $Escenario = $match.Groups[1].Value.Trim().Trim('"')
        if (![IO.Path]::GetExtension($Escenario)) { $Escenario += '.fxml' }
        if (![IO.Path]::IsPathRooted($Escenario)) { $Escenario = Join-Path (Split-Path $PreparExe) $Escenario }
    }
    Assert-File $Escenario
    $settings = Get-ChildItem (Join-Path $env:LOCALAPPDATA 'MobiFlight') -Filter user.config -Recurse |
        Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if (!$settings) { throw 'No se encontro user.config de MobiFlight. Abrilo una vez y volve a ejecutar.' }
    $mobiXml = New-Object System.Xml.XmlDocument
    $mobiXml.PreserveWhitespace = $true
    $mobiXml.Load($settings.FullName)
    if (!$mobiXml.SelectSingleNode('/configuration/userSettings/MobiFlight.Properties.Settings')) { throw 'Configuracion MobiFlight no reconocida.' }
    $yoke = Find-YokePort $FirmwareEsperado
    $Puerto = $yoke.Port
    Write-Host "Pico 2 del yoke detectada: $Puerto | $($yoke.Firmware) | $($yoke.InstanceId)"
    $mobi = Get-Process -Name MFConnector -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($mobi -and !(Test-MobiIgnoresPort $mobiXml $Puerto)) {
        throw "MobiFlight ya esta abierto y $Puerto no figura reservado para el yoke. Cerra MobiFlight y volve a ejecutar el lanzador para actualizar su exclusion automaticamente."
    }
    Write-Host "Prepar3D: $PreparExe"
    Write-Host "MobiFlight: $MobiProyecto"
    Write-Host "Escenario: $Escenario"
    Write-Host "Puente: --port $Puerto --enable --gusts --gust-gain 1"
    Write-Host "Ejecutable: $BridgeExe | Firmware esperado: $FirmwareEsperado"
    if ($Comprobar) { Write-Host 'Comprobacion OK. No se abrieron programas ni se modificaron configuraciones.'; exit 0 }
    if (Get-Process -Name YokeSimConnectBridge -ErrorAction SilentlyContinue) { Write-Host 'El puente ya esta abierto. No se iniciara otro.'; exit 0 }

    if (!$mobi) {
        # Reservar el COM detectado para el puente, conservando exclusiones del usuario.
        $ignoredNode = $mobiXml.SelectSingleNode("//setting[@name='IgnoredComPortsList']/value")
        $ports = @()
        if ($ignoredNode) { $ports = @($ignoredNode.InnerText.Split(',') | ForEach-Object { $_.Trim() } | Where-Object { $_ }) }
        if ($ports -notcontains $Puerto) { $ports += $Puerto }
        Backup-Once $settings.FullName
        Set-MobiValue $mobiXml 'IgnoreComPorts' 'True'
        Set-MobiValue $mobiXml 'IgnoredComPortsList' ($ports -join ',')
        Set-MobiValue $mobiXml 'AutoRun' 'True'
        $mobiXml.Save($settings.FullName)
        Write-Host "MobiFlight configurado: AutoRun y $Puerto reservado para el puente."
    } else {
        Write-Host 'MobiFlight ya esta abierto: se conserva su proyecto y estado Run actuales.'
    }
    if (!$prepar) {
        # Saltar la pantalla de seleccion y cargar el vuelo predeterminado.
        if ($cfgText -notmatch '(?im)^SHOW_SCENARIO_WINDOW=') { throw 'Falta SHOW_SCENARIO_WINDOW en Prepar3D.cfg.' }
        $newCfg = [regex]::Replace($cfgText, '(?im)^SHOW_SCENARIO_WINDOW=[^\r\n]*', 'SHOW_SCENARIO_WINDOW=0')
        if ($newCfg -ne $cfgText) {
            Backup-Once $PreparCfg
            [IO.File]::WriteAllText($PreparCfg, $newCfg, [Text.Encoding]::Default)
        }
    }
    if ($SoloPreparar) { Write-Host 'Configuraciones preparadas. No se iniciaron programas ni el puente.'; exit 0 }
    if (!$mobi) {
        Write-Host 'Abriendo MobiFlight en modo automatico...'
        $mobi = Start-Process -FilePath $MobiExe -WorkingDirectory (Split-Path $MobiExe) -ArgumentList ('/cfg "' + $MobiProyecto + '" /autoRun') -WindowStyle Minimized -PassThru
        Start-Sleep -Seconds 3
        if ($mobi.HasExited) { throw 'MobiFlight se cerro al iniciar. Revisa su log.txt.' }
    }
    if (!$prepar) {
        Write-Host 'Abriendo Prepar3D y cargando el escenario...'
        $prepar = Start-Process -FilePath $PreparExe -WorkingDirectory (Split-Path $PreparExe) -ArgumentList ('"-fxml:' + $Escenario + '"') -PassThru
    }
    Write-Host 'Esperando avion en tierra, sin pausa y fuera de Slew. P permite quitar la pausa.'
    & $WaitExe $EsperaMaximaSegundos $prepar.Id
    if ($LASTEXITCODE -ne 0) { throw "No se confirmo el vuelo listo (codigo $LASTEXITCODE). Comprueba carga del escenario, pausa y SimConnect." }
    if (!(Get-Process -Name MFConnector -ErrorAction SilentlyContinue)) { throw 'MobiFlight se cerro. El puente no se iniciara.' }
    if (Get-Process -Name YokeSimConnectBridge -ErrorAction SilentlyContinue) { throw 'Se abrio otro puente durante la espera.' }
    # Detectar desconexion/cambio durante la carga del simulador antes de abrir el puente.
    $currentYoke = Find-YokePort $FirmwareEsperado
    if ($currentYoke.Port -ne $Puerto -or $currentYoke.InstanceId -ne $yoke.InstanceId) {
        throw "La Pico cambio de conexion durante la espera ($Puerto -> $($currentYoke.Port)). Cerra MobiFlight y reinicia el lanzador para reservar el nuevo puerto."
    }
    Write-Host 'Iniciando puente. Mantene esta ventana abierta mientras usas el simulador.'
    Push-Location $BridgeDir
    try {
        & $BridgeExe --port $Puerto --enable --gusts --gust-gain 1
        $bridgeCode = $LASTEXITCODE
    } finally { Pop-Location }
    if ($bridgeCode -ne 0) { throw "El puente termino con codigo $bridgeCode. Revisa $BridgeDir\bridge.log. $Puerto debe estar libre y tener el firmware $FirmwareEsperado." }
    Write-Host 'El puente finalizo.'
} catch {
    Write-Host ("ERROR: " + $_.Exception.Message) -ForegroundColor Red
    exit 1
} finally {
    if ($transcript) { Stop-Transcript | Out-Null }
    if ($locked) { $mutex.ReleaseMutex() }
    $mutex.Dispose()
}
