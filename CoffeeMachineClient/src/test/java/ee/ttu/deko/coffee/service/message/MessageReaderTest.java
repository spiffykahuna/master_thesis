package ee.ttu.deko.coffee.service.message;

import ee.ttu.deko.coffee.jsonrpc.RPCNotification;
import ee.ttu.deko.coffee.jsonrpc.RPCRequest;
import ee.ttu.deko.coffee.jsonrpc.RPCResponse;
import org.junit.Before;
import org.junit.Test;

import java.io.Reader;
import java.io.StringReader;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;

import static org.junit.Assert.assertEquals;

public class MessageReaderTest {
    List<String> validList;
    private Reader inputReader;

    @Before
    public void setUp() throws Exception {
        validList = new ArrayList<String>();
    }

    class TestHandler implements MessageHandler {
        public int handleCounter = 0;
        public List<String> msgList = new ArrayList<String>();
        @Override
        public void handleMessage(String msg) {
            msgList.add(new String(msg));
            handleCounter++;
        }

        @Override
        public void handleMessage(RPCRequest request) { }

        @Override
        public void handleMessage(RPCNotification notification) { }

        @Override
        public void handleMessage(RPCResponse response) { }
    };

    public void execReader(MessageReader reader) throws InterruptedException {
        reader.start();
        Thread.sleep(1000);
        reader.stop();
        while(reader.isAlive());
    }

    @Test
    public void messageHandlerReceivesRightMessages() throws Exception {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 10; i++) {
            String msg = String.format("Message %d\n", i);
            sb.append(toNetString(msg));
            validList.add(msg);
        }

        inputReader = new StringReader(sb.toString());

        TestHandler handler = new TestHandler();
        MessageReader  reader = new MessageReader(inputReader, handler);

        execReader(reader);

        assertEquals(10, (Object) handler.handleCounter);
        assertEquals(validList, handler.msgList);
    }

    @Test
    public void readerAcceptsStringsWithCharactersBetweenMessages() throws Exception {
        StringBuilder sb = new StringBuilder();
        for (int i = 9; i < 126; i++) {
            if(i > 47 && i < 58) continue;
            String msg = String.format("Message %d", i);
            sb.append(toNetString(msg)).append(Character.valueOf((char) i));
            validList.add(msg);
        }

        inputReader = new StringReader(sb.toString());

        TestHandler handler = new TestHandler();
        MessageReader  reader = new MessageReader(inputReader, handler);

        execReader(reader);

        assertEquals(validList.size(), (Object) handler.handleCounter);
        assertEquals(validList, handler.msgList);

    }

    private String toNetString(String msg) throws UnsupportedEncodingException {
        StringBuilder sb = new StringBuilder(msg.length() + 4);
        sb.append(msg.getBytes("UTF-8").length)
                .append(':')
                .append(msg)
                .append(',');
        return sb.toString();
    }

    @Test
    public void readerFiltersCarriageReturnCharacters() throws Exception {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 10; i++) {
            String msg = String.format("Message %d\n", i);
            sb.append(toNetString(msg + "\r"));
            validList.add(msg);
        }
        for (int i = 0; i < 10; i++) {
            String msg = String.format("Message %d", i);
            sb.append(toNetString(msg + "\r\n"));
            validList.add(msg + "\n");
        }

        inputReader = new StringReader(sb.toString());

        TestHandler handler = new TestHandler();
        MessageReader  reader = new MessageReader(inputReader, handler);

        execReader(reader);

        assertEquals(validList.size(), (Object) handler.handleCounter);
        assertEquals(validList, handler.msgList);
    }

    @Test
    public void readerReceivesJsonMessages() throws Exception {
        inputReader = new StringReader("59:{\"id\":1375391318322,\"method\":\"system.help\",\"jsonrpc\":\"2.0\"},");
        validList.add("{\"id\":1375391318322,\"method\":\"system.help\",\"jsonrpc\":\"2.0\"}");

        TestHandler handler = new TestHandler();
        MessageReader  reader = new MessageReader(inputReader, handler);

        execReader(reader);

        assertEquals(1, (Object) handler.handleCounter);
        assertEquals(validList, handler.msgList);
    }
}
