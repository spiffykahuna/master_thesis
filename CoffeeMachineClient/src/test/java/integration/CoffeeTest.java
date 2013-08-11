package integration;

import ee.ttu.deko.coffee.service.JsonRpcCoffeeMachineService;
import ee.ttu.deko.coffee.service.domain.Product;
import ee.ttu.deko.coffee.service.domain.ServiceContract;
import org.junit.*;

import java.util.List;
import java.util.Map;

import static java.lang.Thread.currentThread;
import static java.lang.Thread.sleep;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

//@Ignore
public class CoffeeTest {
    private static final long DEFAULT_SERVER_TIMEOUT = 10000;

    static ComPortRW comRW;
    static Thread comHandler;

    static JsonRpcCoffeeMachineService service;

    @Before
    public void setUp() throws Exception {
        service = new JsonRpcCoffeeMachineService();
        service.connect(comRW.portReader, comRW.portWriter);
        service.start();
    }

    @After
    public void tearDown() throws Exception {
        if(service.isRunning())     service.stop();
        if(service.isConnected())   service.disconnect();
        sleep(1000);
    }

    @BeforeClass
    public static void setUpClass() throws Exception {
        comRW = new ComPortRW(ComPortRW.DEFAULT_COM_PORT);
//        comHandler = new Thread(comRW, ComPortRW.class.getSimpleName());
//        comHandler.start();
    }

    @AfterClass
    public static void tearDownClass() throws Exception {
//        comHandler.interrupt();
//        comHandler.join();


        comRW.close();
        sleep(1000);
        if(service.isRunning())     service.stop();
        if(service.isConnected())   service.disconnect();
    }


    @Test(timeout = DEFAULT_SERVER_TIMEOUT)
    public void serviceReturnsContract() throws Exception {
        ServiceContract contract = service.getServiceContract();
        assertNotNull(contract);
        assertTrue(contract instanceof ServiceContract);
        assertTrue(contract.containsKey("id"));
        assertTrue(contract.containsKey("version"));
        assertTrue(contract.containsKey("operations"));
        assertTrue(contract.containsKey("description"));
        assertTrue(contract.containsKey("$schema"));
    }

    @Test(timeout = DEFAULT_SERVER_TIMEOUT*10)
    public void serviceReturnsProductList() throws Exception {
        for(int i = 0; i < 10; i++) {
            List<Product> products = service.getProducts();
            assertNotNull(products);
            assertTrue(products.size() > 0);
            sleep(500);
        }
    }

    @Test
    public void serviceReturnsInfo() throws Exception {
        Map<String, Object> info = service.getInfo();
        assertNotNull(info);
        assertTrue(info.containsKey("machineName"));
        assertNotNull(info.get("machineName"));
        assertEquals(String.class, info.get("machineName").getClass());

        assertTrue(info.containsKey("machineFirmvareVersion"));
        assertNotNull(info.get("machineFirmvareVersion"));
        assertEquals(String.class, info.get("machineFirmvareVersion").getClass());
    }

    @Test
    public void serviceReturnsOrderProductStatus() throws Exception {
        Product.Status status = service.orderProduct(3);
        assertNotNull(status);
        assertTrue(status instanceof Product.Status);

    }

    @Test
    public void coffeeGoodScenario() throws Exception {
        Product.Status status = service.orderProduct(3);
        assertNotNull(status);
        assertTrue(status instanceof Product.Status);
        assertEquals(Product.Status.PRODUCT_STATUS_STARTED, status);

        sleep(3000);

        status = service.getProductStatus(3);
        assertNotNull(status);
        assertTrue(status instanceof Product.Status);
        assertEquals(Product.Status.PRODUCT_STATUS_IN_PROGRESS, status);

        sleep(15000);

        status = service.getProductStatus(3);
        assertNotNull(status);
        assertTrue(status instanceof Product.Status);
        assertEquals(Product.Status.PRODUCT_STATUS_READY, status);
    }

    @Test
    public void coffeeCancelScenario() throws Exception {
        Product.Status status = service.orderProduct(3);
        assertNotNull(status);
        assertTrue(status instanceof Product.Status);
        assertEquals(Product.Status.PRODUCT_STATUS_STARTED, status);

        sleep(3000);

        status = service.getProductStatus(3);
        assertNotNull(status);
        assertTrue(status instanceof Product.Status);
        assertEquals(Product.Status.PRODUCT_STATUS_IN_PROGRESS, status);

        sleep(3000);

        status = service.cancelProduct(3);
        assertNotNull(status);
        assertTrue(status instanceof Product.Status);
        assertEquals(Product.Status.PRODUCT_STATUS_CANCELLED, status);

        sleep(3000);

        status = service.getProductStatus(3);
        assertNotNull(status);
        assertTrue(status instanceof Product.Status);
        assertEquals(Product.Status.PRODUCT_STATUS_CANCELLED, status);
    }
}
