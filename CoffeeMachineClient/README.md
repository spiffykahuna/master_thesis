This is common client library. 

AbstractCoffeeMachineService class is general service implementation.
API uses java.io.Reader and java.io.Writer abstraction and handles character streams only. 
It is line oriented ( each message should end with \n in order to be processed).
Library user must connect service with appropriate character streams using void connect(Reader inputReader, Writer outputWriter) method.
