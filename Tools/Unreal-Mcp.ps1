param(
    [string]$Url = 'http://127.0.0.1:8000/mcp',
    [string]$Method = 'tools/list',
    [string]$ParametersJson = '{}'
)
$ErrorActionPreference = 'Stop'
$mcpHeaders = @{ Accept = 'application/json, text/event-stream' }
function Read-McpResponse($Response) {
    $body = [string]$Response.Content
    if ($body.TrimStart().StartsWith('{')) { return ($body | ConvertFrom-Json) }
    foreach ($line in ($body -split "`n")) {
        if ($line.StartsWith('data:')) {
            $candidate = $line.Substring(5).Trim() | ConvertFrom-Json
            if ($null -ne $candidate.id) { return $candidate }
        }
    }
    throw 'No JSON-RPC reply in MCP response.'
}
function Send-McpRequest([string]$RequestMethod, $Parameters, $RequestId) {
    $request = @{ jsonrpc = '2.0'; method = $RequestMethod; params = $Parameters }
    if ($null -ne $RequestId) { $request.id = $RequestId }
    Invoke-WebRequest -Uri $Url -Method Post -Headers $mcpHeaders -ContentType 'application/json' -Body ($request | ConvertTo-Json -Depth 50 -Compress) -TimeoutSec 60 -UseBasicParsing
}
$response = Send-McpRequest 'initialize' @{ protocolVersion = '2025-03-26'; capabilities = @{}; clientInfo = @{ name = 'Daedalus-environment-check'; version = '1.0' } } 1
$init = Read-McpResponse $response
if ($init.error) { throw ($init.error | ConvertTo-Json -Compress) }
if ($response.Headers['Mcp-Session-Id']) { $mcpHeaders['Mcp-Session-Id'] = ($response.Headers['Mcp-Session-Id'] -join '') }
$mcpHeaders['MCP-Protocol-Version'] = $init.result.protocolVersion
$null = Send-McpRequest 'notifications/initialized' @{} $null
$reply = Read-McpResponse (Send-McpRequest $Method ($ParametersJson | ConvertFrom-Json) 2)
if ($reply.error) { throw ($reply.error | ConvertTo-Json -Compress) }
if ($reply.result.isError) { throw ($reply.result | ConvertTo-Json -Depth 50 -Compress) }
$reply.result | ConvertTo-Json -Depth 70
