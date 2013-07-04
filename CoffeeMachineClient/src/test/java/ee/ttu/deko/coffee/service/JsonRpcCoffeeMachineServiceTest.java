package ee.ttu.deko.coffee.service;

import com.thetransactioncompany.jsonrpc2.JSONRPC2Error;
import ee.ttu.deko.coffee.jsonrpc.JsonRpc2_0Spec;
import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.jsonrpc.RPCResponse;
import ee.ttu.deko.coffee.service.domain.ServiceContract;
import ee.ttu.deko.coffee.service.request.RequestProcessor;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.io.*;
import java.util.HashMap;

import static org.hamcrest.core.StringContains.containsString;
import static org.junit.Assert.*;

public class JsonRpcCoffeeMachineServiceTest {
    JsonRpcCoffeeMachineService service;
    Reader inputReader;
    Writer inputWriter;

    Reader outputReader;
    Writer outputWriter;

    @Before
    public void setUp() throws Exception {
        /*
        inputWriter --> inputReader -->  service --> outputWriter --> outputReader;
        */
        inputWriter = new PipedWriter();
        inputReader = new PipedReader((PipedWriter) inputWriter);

        outputWriter = new PipedWriter();
        outputReader = new PipedReader((PipedWriter) outputWriter);

        service = new JsonRpcCoffeeMachineService();
    }

    @After
    public void tearDown() throws Exception {
        if(service.isRunning())     service.stop();
        if(service.isConnected())   service.disconnect();
    }

    @Test
    public void testStart() throws Exception {
        try {
            service.start();
            fail("Service should not start. Readers/Writers streams are not connected");
        } catch(IllegalStateException e) {
            // skip
        }

        service.connect(inputReader, outputWriter);
        assertTrue(service.isConnected());

        service.start();
        assertTrue(service.isRunning());

        service.start();
        assertTrue(service.isRunning());

        service.stop();
        assertFalse(service.isRunning());

        service.disconnect();
        assertFalse(service.isConnected());

        service.connect(inputReader, outputWriter);
        assertTrue(service.isConnected());

        service.start();
        assertTrue(service.isRunning());

        service.stop();
        Thread.sleep(2000);
        assertFalse(service.isRunning());
    }

    @Test
    public void testConnect() throws Exception {
        try {
            service.connect(null, outputWriter);
            fail("You should not get here!!! Get out!!!");
        } catch(Exception e) {
            // this is a right place to be
        }

        try {
            service.connect(inputReader, null);
            fail("You should not get here!!! Get out!!!");
        } catch(Exception e) {
            // this is a right place to be
        }

        try {
            service.connect(null, null);
            fail("You should not get here!!! Get out!!!");
        } catch(Exception e) {
            // this is a right place to be
        }

        service.connect(inputReader, outputWriter);
        assertTrue(service.isConnected());
    }

    @Test
    public void testDisconnect() throws Exception {
        service.connect(inputReader, outputWriter);
        assertTrue(service.isConnected());

        service.disconnect();

        service.connect(inputReader, outputWriter);
        assertTrue(service.isConnected());

        service.start();
        assertTrue(service.isRunning());

        try {
            service.disconnect();
            fail("Service should be stopped before disconnecting.");
        } catch (IllegalStateException ise) {
            // silence
        }

        service.stop();
        Thread.sleep(1000);
        assertFalse(service.isRunning());

        service.disconnect();
        assertFalse(service.isConnected());
    }

    @Test
    public void testGetServiveContract() throws Exception {
        service.connect(inputReader, outputWriter);

        final String contractMethodName =  "system.help";

        final String[] requestJson = new String[1];

        service.setRequestProcessor(new RequestProcessor() {
            @Override
            public Object processRequest(Object request) {
                RPCRequest rpcRequest = (RPCRequest) request;
                assertNotNull(rpcRequest);

                requestJson[0] = rpcRequest.toString();
                assertEquals(contractMethodName, rpcRequest.getMethod());
                JsonRpc2_0Spec.checkId(rpcRequest.getID());

                RPCResponse response = new RPCResponse(rpcRequest.getID());
                ServiceContract contract = new ServiceContract();
                contract.setId("some_service_id");
                contract.setDescription("some_service_description");
                contract.setVersion("some_service_version");
                contract.setOperations( new HashMap<String, Object>() {{
                    put("someMethod1", "someMethod1_declaration");
                    put("someMethod2", "someMethod2_declaration");
                    put("someMethod3", "someMethod3_declaration");
                }});

                contract.setDefinitions(new HashMap<String, Object>() {{
                    put("someDef1", "someDef1_declaration");
                    put("someDef2", "someDef2_declaration");
                    put("someDef3", "someDef3_declaration");
                }});
                response.setResult(contract);
                return response;
            }

            @Override
            public RequestProcessor cloneProcessor() {
                return this;
            }
        });

        ServiceContract contract = service.getServiceContract();

        assertEquals("some_service_id", contract.getId());
        assertEquals("some_service_description", contract.getDescription());
        assertEquals("some_service_version", contract.getVersion());
        assertEquals(3, contract.getOperations().size());
        assertTrue(contract.getOperations().containsKey("someMethod1"));
        assertTrue(contract.getOperations().containsKey("someMethod2"));
        assertTrue(contract.getOperations().containsKey("someMethod3"));

        assertEquals(3, contract.getDefinitions().size());
        assertTrue(contract.getDefinitions().containsKey("someDef1"));
        assertTrue(contract.getDefinitions().containsKey("someDef2"));
        assertTrue(contract.getDefinitions().containsKey("someDef3"));

        assertTrue(requestJson[0].contains(contractMethodName));

        // request is null
        service.setRequestProcessor( new RequestProcessor() {
            @Override
            public Object processRequest(Object request) {
                return null;
            }

            @Override
            public RequestProcessor cloneProcessor() {
                return this;
            }
        });

        contract = service.getServiceContract();
        assertNull(contract);

        // processor returns wrong response(This is not valid behaviour).  id do not match
        service.setRequestProcessor( new RequestProcessor() {
            @Override
            public Object processRequest(Object request) {
                RPCRequest rpcRequest = (RPCRequest) request;
                assertNotNull(rpcRequest);

                Object responseId = null;

                Object id = rpcRequest.getID();
                if(id instanceof  Number) {
                    responseId = ((Number) id).longValue() + 1L;
                }

                if(id instanceof String) {
                    responseId = ((String) id) + "1";
                }
                if(responseId == null) responseId = Long.valueOf(Long.MAX_VALUE);

                RPCResponse response = new RPCResponse("some_result", responseId);
                assertNotNull(response);
                return response;
            }

            @Override
            public RequestProcessor cloneProcessor() {
                return this;
            }
        });

        contract = service.getServiceContract();
        assertNull(contract);

        // processor returns wrong response(This is not valid behaviour).  id is null
        service.setRequestProcessor( new RequestProcessor() {
            @Override
            public Object processRequest(Object request) {
                RPCRequest rpcRequest = (RPCRequest) request;
                assertNotNull(rpcRequest);


                RPCResponse response = new RPCResponse("some_result", null);
                assertNotNull(response);
                return response;
            }

            @Override
            public RequestProcessor cloneProcessor() {
                return this;
            }
        });

        contract = service.getServiceContract();
        assertNull(contract);

        // processor returns different kind of object
        service.setRequestProcessor( new RequestProcessor() {
            @Override
            public Object processRequest(Object request) {
                RPCRequest rpcRequest = (RPCRequest) request;
                assertNotNull(rpcRequest);
                return new String("This should fail. Processor should return only response objects.");
            }

            @Override
            public RequestProcessor cloneProcessor() {
                return this;
            }
        });

        contract = service.getServiceContract();
        assertNull(contract);

        // processor returns error object
        service.setRequestProcessor( new RequestProcessor() {
            @Override
            public Object processRequest(Object request) {
                RPCRequest rpcRequest = (RPCRequest) request;
                assertNotNull(rpcRequest);

                RPCResponse response = new RPCResponse(JSONRPC2Error.METHOD_NOT_FOUND, rpcRequest.getID());
                return response;
            }

            @Override
            public RequestProcessor cloneProcessor() {
                return this;
            }
        });
        contract = service.getServiceContract();
        assertNull(contract);

        service = new JsonRpcCoffeeMachineService();
        service.connect(inputReader, outputWriter);
    }

    @Test
    public void disconnectShouldBeAfterStop() throws Exception {
        service.connect(inputReader, outputWriter);
        service.start();
        try {
            service.disconnect();
            fail("Disconnect should throw exception if it was called before service was stopped");
        } catch (IllegalStateException iseValid) {}
    }

    @Test(timeout = 5000)
    public void getContractWritesResultToStream() throws Exception {
        final JsonRpcCoffeeMachineService jsonService = new JsonRpcCoffeeMachineService();
        jsonService.connect(inputReader, outputWriter);
        jsonService.start();

        new Thread( new Runnable() {
            @Override
            public void run() {
                ServiceContract contract = jsonService.getServiceContract();
            }
        }).start();

        StringBuilder sb = new StringBuilder();
        int c = 0;
        String outValue = "";
        do {
            c = outputReader.read();
            sb.append((char) c);
            outValue = sb.toString();
            int opens = countOf(outValue, '{');
            int closes = countOf(outValue, '}');
            if( (opens != 0) && (closes != 0) && ( (opens + closes) % 2 == 0)) { //TODO implement this control statement in embedded service too
                break;
            }
        } while( c != -1);

        outValue = sb.toString();
        assertThat(outValue, containsString("jsonrpc"));
        assertThat(outValue, containsString("2.0"));
        assertThat(outValue, containsString("id"));
        assertThat(outValue, containsString("method"));
        assertThat(outValue, containsString("system.help"));

    }

    public static int countOf( final String s, final char c ) {
        final char[] chars = s.toCharArray();
        int count = 0;
        for(int i=0; i<chars.length; i++) {
            if (chars[i] == c) {
                count++;
            }
        }
        return count;
    }

    @Test
    public void serviceCannotStartAfterDisconnect() throws Exception {
        service.connect(inputReader, outputWriter);
        service.disconnect();
        try{
            service.start();
            fail("Service should not start after disconnect");
        } catch (IllegalStateException iseValid) {}

        service.connect(inputReader, outputWriter);
        service.start();
        service.stop();
        service.disconnect();
        try{
            service.start();
            fail("Service should not start after disconnect");
        } catch (IllegalStateException iseValid) {}
    }

    @Test
    public void isRunningShouldWorkInEachState() throws Exception {
        assertFalse(service.isRunning());

        service.connect(inputReader, outputWriter);
        assertFalse(service.isRunning());

        service.start();
        assertTrue(service.isRunning());

        service.stop();
        assertFalse(service.isRunning());

        service.disconnect();
        assertFalse(service.isRunning());
    }
}
