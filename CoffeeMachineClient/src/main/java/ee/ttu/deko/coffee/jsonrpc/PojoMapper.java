package ee.ttu.deko.coffee.jsonrpc;

import com.fasterxml.jackson.annotation.JsonAutoDetect;
import com.fasterxml.jackson.annotation.PropertyAccessor;
import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonGenerationException;
import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.core.JsonParseException;
import com.fasterxml.jackson.databind.JsonMappingException;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringWriter;

public class PojoMapper {

    private static ObjectMapper m = new ObjectMapper();
    private static JsonFactory jf = new JsonFactory();

    static {
        m.setVisibility(PropertyAccessor.FIELD, JsonAutoDetect.Visibility.ANY);
    }

    public static <T> Object fromJson(String jsonAsString, Class<T> pojoClass)
            throws JsonMappingException, JsonParseException, IOException {
        return m.readValue(jsonAsString, pojoClass);
    }

    public static <T> Object fromJson(FileReader fr, Class<T> pojoClass)
            throws JsonParseException, IOException
    {
        return m.readValue(fr, pojoClass);
    }

    public static String toJson(Object pojo, boolean prettyPrint)
            throws JsonMappingException, JsonGenerationException, IOException {
        StringWriter sw = new StringWriter();
        JsonGenerator jg = jf.createGenerator(sw);
        if (prettyPrint) {
            jg.useDefaultPrettyPrinter();
        }
        m.writeValue(jg, pojo);
        return sw.toString();
    }

    public static void toJson(Object pojo, FileWriter fw, boolean prettyPrint)
            throws JsonMappingException, JsonGenerationException, IOException {
        JsonGenerator jg = jf.createGenerator(fw);
        if (prettyPrint) {
            jg.useDefaultPrettyPrinter();
        }

        m.writeValue(jg, pojo);
    }
}