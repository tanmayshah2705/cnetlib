@page templates Templates

@brief Template code for quick client/server application.

<ul>
<li>\ref client_template "client_template.c"</li>
<li>\ref server_template "server_template.c"</li>
</ul>

<hr/>

<h2>Demonstration</h2>
<ul>
<li>starting a TCP server</li>
<li>connecting a TCP client</li>
<li>fetching remote IP/port details on both sides</li>
<li>sending a hello message from the client</li>
<li>sending a hello message from the server</li>
<li>checking whether the client is still connected</li>
<li>sending a stop-server request from the client</li>
<li>stopping the server, releasing client/server resources, and exiting the server app</li>
</ul>

<hr/>

<b>Note</b>: After copying a template, configure the server address, port, and any placeholders before running.







@page client_template client_template.c
@brief Minimal client template using the csnl library.

\include client_template.c

@page server_template server_template.c
@brief Minimal server template using the ssnl library.

\include server_template.c
