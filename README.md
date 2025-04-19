# HERMES High-Availability File Transfer System

**HERMES** is a **Highly Available, Fault Tolerant and Flexible** File Transfer System that provides
support for file transfer via multiple protocols.

## Project Setup :-

1. `Active Server`

2. `Backup Server`

3. `Switch Emulator` Cum Watchdog [Auto Detects Server Failure and Switches to Backup]

4. `Client`

5. `Logs`

### Features:
1. Our Servers can Serve only 1 client At a time <br>
2. File Transfer Occurs Over UDP <br>
3. For Resilience We use **Stop_&_Wait** , **Go_Back_N** , **Selective_Repeat**<br> 
[Each Implementation on Seperate Branch]
4. We have also provided **logging support** on the **SERVER** side, 
after each transmission, the transmission details are saved inside
`LOGS/logs.txt`.<br>
To access logs:
<pre>
cd LOGS
open logs.txt
</pre>


## How to Run?

Download the zip. Extract the folders from the zip.
<pre>cd Major-Project</pre>

### To run Server:
i. Go to /SERVER , open a terminal and run 
<pre>cd SERVER
./server_exe "port-number" .</pre>
We recommend using <8000> as the default port. For using new port the configuration 
needs to be changed under the **/SWITCH** directory. 
**SWITCH** instructions follow later below.


ii. Make sure boost library is installed since Server 
requires **Boost** as a third-party dependency.
<br>
To install use the command:
<pre>cd SERVER
sudo apt install libboost-all-dev</pre>

ii. To compile use:
<pre>cd SERVER
g++ server.cpp -o server_exe</pre>

**Step by step process:**
<pre>
cd SERVER
sudo apt install libboost-all-dev
g++ server.cpp -o server_exe
./server_exe 8000 
</pre>


### To run Client:
i. Navigate to /CLIENT, open a terminal.
Run:
<pre><code>./client_exe &lt;server-ip&gt; &lt;switch-port&gt; &lt;read/write choice&gt;</code></pre>
- You can use any &lt;server ip&gt; and &lt;switch-port&gt;. 
- Server-ip and switch port are set to **127.0.0.1** and **8080** by 
default in examples,can be replaced by suitable ip and port.
- You can denote read choice using 'r' symbol and write choice using
'w' symbol. 


ii. User will be asked to enter FILENAME.
To test **Read** and **Write** we have provided some of the Files under 
**/SERVER/TEST** directory and **/CLIENT/TEST**.
<br>
Enter any of the following filepaths:
- TEST/server3.txt- 1MB
- TEST/server4.txt- 500 KB
- TEST/server5.txt- 200 KB
- TEST/server2.txt- 100KB
- TEST/server1.txt- 100KB
- TEST/server.txt- 3KB
- TEST/b.png- 22KB
- TEST/video.webm- 22MB

iii. To compile use:
<pre>g++ client.cpp -o client_exe</pre>

**Step by step process:**
<pre>
#Read from server into client
cd CLIENT
g++ client.cpp -o client_exe
./client_exe  127.0.0.1 8080 r
</pre>
<pre>
#Write to server from client
cd CLIENT
g++ client.cpp -o server_exe
./server_exe 127.0.0.1 8080 w
</pre>

### To run Switch:
i. Go to /SWITCH, open a terminal. Run:
<pre>./build/sw_exe &lt;switch-port&gt; config.txt</pre>
**8080** has been used as a default switch port but any port can be
chosen during the runtime. Just make sure that **CLIENT** connects
to the same port.

ii. **config.txt** contains the following configuration details:
<ol type='a'>
<li>number of retransmissions allowed from the client without 
any response from the server. After which active server instance is 
presumed dead.
</li>
<li> The active server port, set to <strong>8000</strong> by default.</li>
<li> The backup server port, set to <strong>8001</strong> by default.</li>
</ol>

iii. If User wants to run active and backup servers on different ports,
the changes need to be made in the config.txt for the switch to detect it
correctly.

iv. A **makefile** has been provided, so to compile run the command:
<pre>make clean; #cleans the /SWITCH directory
make #recompiles the executable
</pre>

**Step by step process:**
<pre>
cd SWITCH
make clean
make
./build/sw_exe 8080 config.txt
</pre>


