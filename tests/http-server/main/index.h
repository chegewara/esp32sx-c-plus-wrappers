static const char index_html[] = {
"<html>"
"<script>"
"document.addEventListener('DOMContentLoaded', event => {"
""
"    let webSocket = new WebSocket('ws://' + window.location.host + '/ws');"
"    webSocket.onopen = function (event) {"
"        console.log('Connected');"
"        webSocket.send('get stats');"
"    };"
""
"    webSocket.onmessage = function (event) {"
"        console.log('onmessage');"
"    };"
""
"    webSocket.onerror = function (event) {"
"        console.log('onerror');"
"    };"
"});"
"</script>"
};