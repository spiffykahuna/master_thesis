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

        service = new JsonRpcCoffeeMachineService();
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
}
