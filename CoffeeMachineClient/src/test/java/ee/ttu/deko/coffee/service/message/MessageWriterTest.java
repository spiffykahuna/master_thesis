package ee.ttu.deko.coffee.service.message;

import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.service.RPCServiceListener;
import ee.ttu.deko.coffee.service.Service;
import ee.ttu.deko.coffee.service.request.RequestProcessor;
import org.junit.Before;
import org.junit.Test;

import java.io.*;

import static junit.framework.TestCase.assertTrue;
import static org.hamcrest.core.StringContains.containsString;
import static org.junit.Assert.assertThat;

public class MessageWriterTest {
    Reader outputReader;
    Writer outputWriter;
    MessageWriter messageWriter;
    TestService service;

    class TestService implements Service {
        Reader reader;
        MessageWriter writer;

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

        public void writeSomeMsg(String message) {
            RPCRequest request = new RPCRequest(message, 1);
            writer.onRequest(request);
        }
    };

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
        messageWriter.start();
        assertTrue(messageWriter.isAlive());
        messageWriter.stop();
    }

    @Test
    public void writesRightString() throws Exception {
        messageWriter.start();
        Thread.sleep(1000);

        service.writeSomeMsg("some_data");
        String output = readOutput(10);
        assertThat(output, containsString("some_data"));

    }

    private String readOutput(int maxCharsToRead ) throws IOException {
        StringBuilder sb = new StringBuilder();
        char[] buff = new char[1024];
        int c = outputReader.read(buff, 0, maxCharsToRead);
        sb.append(buff, 0, c);
//        while( c != -1) {
//            sb.append( (char) c);
//            c = outputReader.read();
//            if(maxCharsToRead-- < 0) break;
//        }

        return sb.toString();
    }
}
