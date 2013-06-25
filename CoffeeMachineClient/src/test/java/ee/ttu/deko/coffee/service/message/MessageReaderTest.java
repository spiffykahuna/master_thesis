package ee.ttu.deko.coffee.service.message;

import org.junit.Test;

import java.io.BufferedReader;
import java.io.Reader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import static org.junit.Assert.assertEquals;

public class MessageReaderTest {
    @Test
    public void messageHandlerReceivesRightMessages() throws Exception {
        List<String> validList = new ArrayList<String>(10);
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 10; i++) {
            String msg = String.format("Message %d", i);
            sb.append(msg).append("\n");
            validList.add(msg);
        }

        Reader inputReader = new StringReader(sb.toString());

        class TestHandler implements MessageHandler {
            public int handleCounter = 0;
            public List<String> msgList = new ArrayList<String>();
            @Override
            public void handleMessage(String msg) {
                msgList.add(new String(msg));
                handleCounter++;
            }
        };

        TestHandler handler = new TestHandler();
        MessageReader  reader = new MessageReader(inputReader, handler);
        reader.start();
        Thread.sleep(1000);
        reader.close();

        while(reader.isAlive())
        assertEquals(10, (Object) handler.handleCounter);
        assertEquals(validList, handler.msgList);
    }
}
