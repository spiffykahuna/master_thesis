package ee.ttu.deko.coffee.service.message;

import ee.ttu.deko.coffee.jsonrpc.RPCNotification;
import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.service.RPCServiceListener;
import ee.ttu.deko.coffee.service.Service;
import ee.ttu.deko.coffee.service.request.RequestProcessor;
import org.junit.Before;
import org.junit.Test;

import java.io.*;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;

import static org.hamcrest.core.StringContains.containsString;
import static org.junit.Assert.*;

public class MessageWriterTest {
    Reader outputReader;
    Writer outputWriter;
    MessageWriter messageWriter;
    TestService service;

    class TestService implements Service {
        Reader reader;
        MessageWriter writer;

        Set<RPCServiceListener> listeners = new HashSet<RPCServiceListener>();

        @Override
        public void connect(Reader inputReader, Writer outputWriter) {
            this.reader = inputReader;
            this.writer = new MessageWriter(outputWriter, this);
        }

        @Override
        public boolean isConnected() {
            return true;
        }

        @Override
        public void disconnect() {
        }

        @Override
        public synchronized boolean addListener(RPCServiceListener listener) {
            return listeners.add(listener);
        }

        @Override
        public boolean removeListener(RPCServiceListener listener) {
            return listeners.remove(listener);
        }

        @Override
        public void setTimeoutMs(long timeoutMs) {
        }

        @Override
        public long getTimeoutMs() {
            return 0;
        }

        @Override
        public void setRequestProcessor(RequestProcessor requestProcessor) {
        }

        public int writeSomeMsg(String message) {
            RPCRequest request = new RPCRequest(message, 1);
            writer.onRequest(request);
            return request.toString().length();
        }
    }

    @Before
    public void setUp() throws Exception {
        /*
        inputWriter --> inputReader -->  service --> outputWriter --> outputReader;
        */

        outputWriter = new PipedWriter();
        outputReader = new PipedReader((PipedWriter) outputWriter);

        service = new TestService();
        messageWriter = new MessageWriter(outputWriter, service);
        service.writer = messageWriter;
    }

    @Test(expected = IllegalArgumentException.class)
    public void nullArgumentsShouldThrowException() throws Exception {
        new MessageWriter(null, null);
        new MessageWriter(null, new Service() {

            @Override
            public void connect(Reader inputReader, Writer outputWriter) {
            }

            @Override
            public boolean isConnected() {
                return false;
            }

            @Override
            public void disconnect() {
            }

            @Override
            public boolean addListener(RPCServiceListener listener) {
                return false;
            }

            @Override
            public boolean removeListener(RPCServiceListener listener) {
                return false;
            }

            @Override
            public void setTimeoutMs(long timeoutMs) {
            }

            @Override
            public long getTimeoutMs() {
                return 0;
            }

            @Override
            public void setRequestProcessor(RequestProcessor requestProcessor) {
            }
        });
        new MessageWriter(new StringWriter(), null);
    }

    @Test
    public void startingTwiceOrMoreTimesDoesNotAffect() throws Exception {
        messageWriter.start();
        assertTrue(messageWriter.isAlive());
        messageWriter.start();
        assertTrue(messageWriter.isAlive());

        for (int i = 0; i < 10; i++) {
            messageWriter.start();
            assertTrue(messageWriter.isAlive());
            messageWriter.stop();
        }
    }

    @Test
    public void writesRightString() throws Exception {
        messageWriter.start();
        Thread.sleep(100);

        int len = service.writeSomeMsg("some_data");
        String output = readOutput(len);
        assertThat(output, containsString("some_data"));

        messageWriter.stop();
    }

    public String readOutput(int totalCharsToRead ) throws IOException {
        StringBuilder sb = new StringBuilder();
        char[] buff = new char[1024];

        while(sb.length() < totalCharsToRead) {
            int readed = outputReader.read(buff, 0, 1024);
            if(readed == -1) break;
            sb.append(buff, 0, readed);
        }

        return sb.toString();
    }

    @Test
    public void writeAfterStop() throws Exception {
        messageWriter.start();
        String output;
        int len = 0;
        for (int i = 0; i < 100; i++) {
            len = service.writeSomeMsg("some_data");
        }

        output = readOutput(len*100);
        assertThat(output, containsString("some_data"));

        messageWriter.stop();

        // this should not affect
        for (int i = 0; i < 100; i++) {
            len = service.writeSomeMsg("some_data");
        }
    }

    @Test
    public void writerRegistersAsListener() throws Exception {
        assertEquals(0, service.listeners.size());
        messageWriter.start();
        assertEquals(1, service.listeners.size());

        for (int i = 0; i < 100; i++) {
            messageWriter.start();
        }
        assertEquals(1, service.listeners.size());

        messageWriter.stop();
        assertEquals(0, service.listeners.size());

        for (int i = 0; i < 100; i++) {
            messageWriter.stop();
        }
        assertEquals(0, service.listeners.size());

        for (int i = 0; i < 100; i++) {
            messageWriter.start();
            messageWriter.stop();
        }
        assertEquals(0, service.listeners.size());

    }

    @Test(timeout = 2000)
    public void writerAcceptsNotifications() throws Exception {
        RPCNotification notification = new RPCNotification("some_method", new ArrayList<Object>() {{
            add("one");
            add("two");
        }});

        int dataLen = MessageWriter.toNetString(notification.toString()).length();
        messageWriter.start();

        messageWriter.onNotification(notification);

        String output = readOutput(dataLen);

        assertEquals(dataLen, output.length());
        assertThat(output, containsString(notification.toString()));
    }

    @Test
    public void nullMessagesDoNothing() throws Exception {
        // null should not affect
        for (int i = 0; i < 100; i++) {
            messageWriter.onNotification(null);
            messageWriter.onRequest(null);
            messageWriter.onResponse(null);
            messageWriter.onUnknownMessage(null);
        }
    }

    @Test
    public void writerWritesProperNetStrings() throws Exception {
        RPCRequest request = new RPCRequest("some_method_with_ünicode", 1);   /* two byte character  ü */
        String netString = MessageWriter.toNetString(request.toString());
        int netLength = Integer.parseInt(netString.substring(0, netString.indexOf(':')));
        assertNotEquals(request.toString().length(), netLength);

        request = new RPCRequest("some_method_without_unicode", 1);
        netString = MessageWriter.toNetString(request.toString());
        netLength = Integer.parseInt(netString.substring(0, netString.indexOf(':')));
        assertEquals(request.toString().length(), netLength);
    }
}
