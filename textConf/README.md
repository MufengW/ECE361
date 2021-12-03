# Text Conferencing
## Additional Features
### User Registration

### Multi-session
The second feature we added is to enable a client to join/create multiple sessions.

#### How it works:

A logged-in client can create a new session anytime, no matter whether the client is currently in a session or not. When a new session is created, the client will automatically switch to this new session from the old session, and at this point, the client can only send/receive messages from the new session. 

To switch back to the previous sessions, the client just needs to issue a "/joinsession" command, and the server will determine whether the client was in that session before ("first time joining" or "returning to an old session").

Therefore, the client does not need to quit a session before joining a new one.

#### How we implemented it:

We used a session-client map to keep track of the status of what sessions a client has joined, what clients are in a session. The map is updated whenever the client-session status has changed (/createsession, /joinsession, /leavesession, /quit,...).

We used an array to keep track of which session is a client actually in (there can be only one active session) so that when clients in this session are communicating, the message can be only sent within the session.
