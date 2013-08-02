package integration;

import ee.ttu.deko.coffee.service.JsonRpcCoffeeMachineService;
import ee.ttu.deko.coffee.service.domain.ServiceContract;
import org.junit.*;

import static junit.framework.Assert.assertNotNull;


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
    }

    @BeforeClass
    public static void setUpClass() throws Exception {
        comRW = new ComPortRW(ComPortRW.DEFAULT_COM_PORT);
//        comHandler = new Thread(comRW, ComPortRW.class.getSimpleName());
//        comHandler.start();
    }

    @AfterClass
    public static void tearDownClass() throws Exception {
        comHandler.interrupt();
        comHandler.join();

        comRW.close();
    }


//    @Test(timeout = DEFAULT_SERVER_TIMEOUT)
    @Test
    public void serviceReturnsVersion() throws Exception {
        ServiceContract contract = service.getServiceContract();
        assertNotNull(contract);
    }

//    @Test
//    public void testCoffee() throws Exception {
//        comRW.writeCommand("59:{\"id\":1375391318322,\"method\":\"system.help\",\"jsonrpc\":\"2.0\"},");
//
//        Thread.sleep(100000000L);
//    }
}
