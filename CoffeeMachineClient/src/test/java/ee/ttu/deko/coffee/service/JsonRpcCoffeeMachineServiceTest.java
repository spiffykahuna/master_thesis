package ee.ttu.deko.coffee.service;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.io.*;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

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
    }

    @Test
    public void testStart() throws Exception {
        service = new JsonRpcCoffeeMachineService();
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
    }

    @After
    public void tearDown() throws Exception {
        service.stop();
        Thread.sleep(2000);
        assertFalse(service.isRunning());
    }
}
