# Text Conferencing
## Additional Features
### User Registration
The first feature we added is to enable a user to register

#### How it works:

A local file is created to store user credentials. If a username is not found in the credential file, login would fail.
New uses could use `/register <username> <password> <repeat_password> <server_address> <server_port>` to register a new account. The new account will be stored in the server's credential file so the user could still use the credentials to login after a server restart.

#### How we implemented it:

We created a file to store the credentials. The format of the file is similar to CSV where each line is a pair of username and password separated by a comma.
When the server starts, it reads the data from the credential file and store it as an array. The lenght of the array is the same as the number of lines in the credential file. When a user is trying to login, server will check the credential array to verify the username and password. If a user registers, server will put a new pair of username and password in to the exsiting file and then refresh the array based on the updated credential file. Since the credential file is stored on the disk, new credentials will be persisted for future use.

### Multi-session
The second feature we added is to enable a client to join/create multiple sessions.

#### How it works:

A logged-in client can create a new session anytime, no matter whether the client is currently in a session or not. When a new session is created, the client will automatically switch to this new session from the old session, and at this point, the client can only send/receive messages from the new session. 

To switch back to the previous sessions, the client just needs to issue a "/joinsession" command, and the server will determine whether the client was in that session before ("first time joining" or "returning to an old session").

Therefore, the client does not need to quit a session before joining a new one.

#### How we implemented it:

We used a session-client map to keep track of the status of what sessions a client has joined, what clients are in a session. The map is updated whenever the client-session status has changed (/createsession, /joinsession, /leavesession, /quit,...).

We used an array to keep track of which session is a client actually in (there can be only one active session) so that when clients in this session are communicating, the message can be only sent within the session.
