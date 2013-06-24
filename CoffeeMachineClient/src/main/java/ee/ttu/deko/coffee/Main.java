package ee.ttu.deko.coffee;

import ee.ttu.deko.coffee.jsonrpc.PojoMapper;
import ee.ttu.deko.coffee.jsonrpc.RPCRequest;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

public class Main {
     public static class Name {
         private String firstName;
         private String lastName;

         public Name(String first, String last) {
            firstName = first;
            lastName = last;
         }

         String getFirstName() {
             return firstName;
         }

         void setFirstName(String firstName) {
             this.firstName = firstName;
         }

         String getLastName() {
             return lastName;
         }

         void setLastName(String lastName) {
             this.lastName = lastName;
         }
     }
     public static void main(String[] args) {
        System.out.println("hello world!");
         // The remote method to call
        String method = "makePayment";
        // The required parameters to pass
        Map<String,Object> params = new HashMap<String,Object>();
        params.put("recipient", new Name("Penny", "Adams"));
        params.put("amount", 175.05);

        // The mandatory request ID
        String id = "req-001";

        System.out.println("Creating new request with properties :");
        System.out.println("\tmethod     : " + method);
        System.out.println("\tparameters : " + params);
        System.out.println("\tid         : " + id + "\n\n");
        // Create request
        RPCRequest reqOut = new RPCRequest(method, params, id);

         // Convert the POJO to a JSON string
        String pojoAsString = null;
        try {
            pojoAsString = PojoMapper.toJson(params, true);
        } catch (IOException e) {
            e.printStackTrace();
        }
        System.out.println(pojoAsString);
        // Serialise request to JSON-encoded string
        String jsonString = reqOut.toString();
        System.out.println(jsonString);
     }
}
