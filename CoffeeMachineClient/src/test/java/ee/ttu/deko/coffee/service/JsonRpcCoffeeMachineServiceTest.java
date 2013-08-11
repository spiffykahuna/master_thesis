package ee.ttu.deko.coffee.service;

import com.thetransactioncompany.jsonrpc2.JSONRPC2Error;
import com.thetransactioncompany.jsonrpc2.JSONRPC2ParseException;
import ee.ttu.deko.coffee.jsonrpc.JsonRpc2_0Spec;
import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.jsonrpc.RPCResponse;
import ee.ttu.deko.coffee.service.domain.Price;
import ee.ttu.deko.coffee.service.domain.Product;
import ee.ttu.deko.coffee.service.domain.ServiceContract;
import ee.ttu.deko.coffee.service.request.RequestProcessor;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.io.*;
import java.math.BigDecimal;
import java.util.Currency;
import java.util.HashMap;
import java.util.List;

import static org.hamcrest.core.StringContains.containsString;
import static org.junit.Assert.*;

public class JsonRpcCoffeeMachineServiceTest {
    JsonRpcCoffeeMachineService service;
    Reader inputReader;
    Writer inputWriter;

    Reader outputReader;
    Writer outputWriter;

    class TestProcessor implements RequestProcessor {
        long timeoutMs = 1;

        @Override
        public Object processRequest(Object request) {
            return null;
        }

        @Override
        public RequestProcessor cloneProcessor() {
            return this;
        }

        @Override
        public void setTimeoutMs(long timeoutMs) {
            if(timeoutMs < 0) throw new IllegalArgumentException("Timeout should be positive. Wrong value specified");
            this.timeoutMs = timeoutMs;
        }

        @Override
        public long getTimeoutMs() {
            return timeoutMs;
        }
    }

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
        service.setTimeoutMs(1); // to speedUp tests
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

        service.setRequestProcessor(new TestProcessor() {
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

                contract.put("$schema", "http://json-schema.org/draft-04/schema#");
                response.setResult(contract);
                return response;
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
        service.setRequestProcessor( new TestProcessor() {
            @Override
            public Object processRequest(Object request) {
                return null;
            }
        });

        contract = service.getServiceContract();
        assertNull(contract);

        // processor returns wrong response(This is not valid behaviour).  id do not match
        service.setRequestProcessor( new TestProcessor() {
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
        });

        contract = service.getServiceContract();
        assertNull(contract);

        // processor returns wrong response(This is not valid behaviour).  id is null
        service.setRequestProcessor( new TestProcessor() {
            @Override
            public Object processRequest(Object request) {
                RPCRequest rpcRequest = (RPCRequest) request;
                assertNotNull(rpcRequest);


                RPCResponse response = new RPCResponse("some_result", null);
                assertNotNull(response);
                return response;
            }
        });

        contract = service.getServiceContract();
        assertNull(contract);

        // processor returns different kind of object
        service.setRequestProcessor( new TestProcessor() {
            @Override
            public Object processRequest(Object request) {
                RPCRequest rpcRequest = (RPCRequest) request;
                assertNotNull(rpcRequest);
                return new String("This should fail. Processor should return only response objects.");
            }
        });

        contract = service.getServiceContract();
        assertNull(contract);

        // processor returns error object
        service.setRequestProcessor( new TestProcessor() {
            @Override
            public Object processRequest(Object request) {
                RPCRequest rpcRequest = (RPCRequest) request;
                assertNotNull(rpcRequest);

                RPCResponse response = new RPCResponse(JSONRPC2Error.METHOD_NOT_FOUND, rpcRequest.getID());
                return response;
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

    @Test(timeout = 2000)
    public void getServiceContractReturnsNullOnTimeout() throws Exception {
        service.connect(inputReader, outputWriter);
        service.setTimeoutMs(500);
        service.start();

        // just start receiving data while nobody is writing to inputReader character stream
        ServiceContract contract = service.getServiceContract();
        assertNull(contract);

    }

    @Test
    public void serviceReceivesProductList() throws Exception {
        service.connect(inputReader, outputWriter);

        service.setRequestProcessor(new TestProcessor(){
            @Override
            public Object processRequest(Object request) {
                assertTrue(request instanceof RPCRequest);
                RPCRequest req = (RPCRequest) request;

                assertFalse(req.getMethod().isEmpty());
                assertTrue(req.getID() instanceof Number);

                RPCResponse response = null;
                try {
                    response = RPCResponse.parse("{\"id\":3,\"result\":[{\"id\":4,\"price\":{\"amount\":1.0,\"currency\":\"EUR\"},\"name\":\"Cappuccino\"},{\"id\":20,\"price\":{\"amount\":1.85,\"currency\":\"EUR\"},\"name\":\"2 cappuccini\"},{\"id\":3,\"price\":{\"amount\":0.45,\"currency\":\"EUR\"},\"name\":\"Coffee\"},{\"id\":19,\"price\":{\"amount\":0.9,\"currency\":\"EUR\"},\"name\":\"2 coffees\"},{\"id\":2,\"price\":{\"amount\":1.2,\"currency\":\"EUR\"},\"name\":\"Espresso\"},{\"id\":18,\"price\":{\"amount\":2.4,\"currency\":\"EUR\"},\"name\":\"2 espresso\"},{\"id\":6,\"price\":{\"amount\":1.35,\"currency\":\"EUR\"},\"name\":\"Macchiato\"},{\"id\":22,\"price\":{\"amount\":2.7,\"currency\":\"EUR\"},\"name\":\"2 macchiato\"},{\"id\":13,\"price\":{\"amount\":0.1,\"currency\":\"EUR\"},\"name\":\"Hot Water\"},{\"id\":7,\"price\":{\"amount\":1.5,\"currency\":\"EUR\"},\"name\":\"Latte Macchiato\"},{\"id\":23,\"price\":{\"amount\":3.0,\"currency\":\"EUR\"},\"name\":\"2 latte macchiato\"},{\"id\":10,\"price\":{\"amount\":0.25,\"currency\":\"EUR\"},\"name\":\"1 portion milk\"},{\"id\":26,\"price\":{\"amount\":0.5,\"currency\":\"EUR\"},\"name\":\"2 portions milk\"},{\"id\":8,\"price\":{\"amount\":0.25,\"currency\":\"EUR\"},\"name\":\"1 portion milk foam\"},{\"id\":24,\"price\":{\"amount\":0.5,\"currency\":\"EUR\"},\"name\":\"2 portions milk foam\"},{\"id\":5,\"price\":{\"amount\":1.5,\"currency\":\"EUR\"},\"name\":\"Caffe latte\"},{\"id\":21,\"price\":{\"amount\":3.0,\"currency\":\"EUR\"},\"name\":\"2 caffe latte\"},{\"id\":12,\"price\":{\"amount\":1.5,\"currency\":\"EUR\"},\"name\":\"Jug of coffee\"},{\"id\":1,\"price\":{\"amount\":1.4,\"currency\":\"EUR\"},\"name\":\"Ristretto\"},{\"id\":17,\"price\":{\"amount\":2.8,\"currency\":\"EUR\"},\"name\":\"2 ristretto\"}],\"jsonrpc\":\"2.0\"}");
                    response.setID(req.getID());
                } catch (JSONRPC2ParseException e) {
                    fail("Unable to create response object");
                }
                assertNotNull(response);
                assertNotNull(response.getResult());
                return response;
            }
        });

        service.start();

        List<Product> products = service.getProducts();
        assertNotNull(products);

        assertEquals(20, products.size());
        assertTrue(products.contains(new Product(4, "Cappuccino", new Price(BigDecimal.valueOf(1.0), Currency.getInstance("EUR")))));
        assertTrue(products.contains(new Product(3, "Coffee", new Price(BigDecimal.valueOf(0.45), Currency.getInstance("EUR")))));
        assertTrue(products.contains(new Product(21, "2 caffe latte", new Price(BigDecimal.valueOf(3.0), Currency.getInstance("EUR")))));
    }

    @Test
     public void orderProductReturnsValidStatus() throws Exception {
        service.connect(inputReader, outputWriter);

        service.setRequestProcessor(new TestProcessor(){
            @Override
            public Object processRequest(Object request) {
                assertTrue(request instanceof RPCRequest);
                RPCRequest req = (RPCRequest) request;

                assertFalse(req.getMethod().isEmpty());
                assertTrue(req.getID() instanceof Number);

                RPCResponse response = new RPCResponse(Product.Status.PRODUCT_STATUS_STARTED.name(), req.getID());

                assertNotNull(response);
                assertNotNull(response.getResult());
                return response;
            }
        });

        service.start();

        Product.Status status = service.orderProduct(4);
        assertNotNull(status);
        assertEquals(Product.Status.PRODUCT_STATUS_STARTED, status);
    }

    @Test
    public void cancelProductReturnsValidStatus() throws Exception {
        service.connect(inputReader, outputWriter);

        service.setRequestProcessor(new TestProcessor(){
            @Override
            public Object processRequest(Object request) {
                assertTrue(request instanceof RPCRequest);
                RPCRequest req = (RPCRequest) request;

                assertFalse(req.getMethod().isEmpty());
                assertTrue(req.getID() instanceof Number);

                RPCResponse response = new RPCResponse(Product.Status.PRODUCT_STATUS_CANCELLED.name(), req.getID());

                assertNotNull(response);
                assertNotNull(response.getResult());
                return response;
            }
        });

        service.start();

        Product.Status status = service.cancelProduct(4);
        assertNotNull(status);
        assertEquals(Product.Status.PRODUCT_STATUS_CANCELLED, status);
    }

    @Test
    public void getProductStatusReturnsValidStatus() throws Exception {
        service.connect(inputReader, outputWriter);

        service.setRequestProcessor(new TestProcessor(){
            @Override
            public Object processRequest(Object request) {
                assertTrue(request instanceof RPCRequest);
                RPCRequest req = (RPCRequest) request;

                assertFalse(req.getMethod().isEmpty());
                assertTrue(req.getID() instanceof Number);

                RPCResponse response = new RPCResponse(Product.Status.PRODUCT_STATUS_IN_PROGRESS.name(), req.getID());

                assertNotNull(response);
                assertNotNull(response.getResult());
                return response;
            }
        });

        service.start();

        Product.Status status = service.getProductStatus(4);
        assertNotNull(status);
        assertEquals(Product.Status.PRODUCT_STATUS_IN_PROGRESS, status);
    }
}
