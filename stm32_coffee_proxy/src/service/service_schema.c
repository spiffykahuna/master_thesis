#include "service_schema.h"


const char *SERVICE_SCHEMA = "{\"operations\":{\"machine.getProducts\":{\"params\":null,\"returns\":{\"minItems\":0,\"items\":{\"$ref\":\"#/definitions/Product\"},\"uniqueItems\":true,\"type\":\"array\",\"description\":\"Coffee machine version info\"},\"type\":\"method\",\"description\":\"Get list of products that current machine can prepare.\"},\"system.getLimits\":{\"params\":null,\"returns\":{\"required\":[\"jsonMaxDocLength\",\"maxIncomeMessages\"],\"type\":\"object\",\"description\":\"Limits of JSON-RPC server\",\"properties\":{\"jsonMaxDocLength\":{\"minimum\":0,\"type\":\"number\",\"description\":\"Maximum length of a JSON document string that could be sent to the server\"},\"maxIncomeMessages\":{\"minimum\":1,\"type\":\"number\",\"description\":\"Maximum simultaneous messages that server could process\"}}},\"type\":\"method\",\"description\":\"Get limits of physical capabilities of a sistem.\"},\"machine.getInfo\":{\"params\":null,\"returns\":{\"required\":[\"machineName\",\"machineFirmvareVersion\"],\"type\":\"object\",\"description\":\"Coffee machine version info\",\"properties\":{\"machineName\":{\"type\":\"string\"},\"machineFirmvareVersion\":{\"type\":\"string\"}}},\"type\":\"method\",\"description\":\"Get information about current coffee machine. This returns some machine parameters\"},\"machine.orderProduct\":{\"params\":[{\"$ref\":\"#/definitions/Product\"}],\"returns\":{\"enum\":[\"PRODUCT_STATE_STARTED\",\"PRODUCT_STATE_IN_PROGRESS\",\"PRODUCT_STATE_FINISHED\"],\"description\":\"Current status of product\"},\"type\":\"method\",\"description\":\"Order product of your interest. Specify product\"},\"system.help\":{\"params\":null,\"returns\":{\"type\":\"string\",\"description\":\"Full service description\"},\"type\":\"method\",\"description\":\"Prints this help document.\"}},\"description\":\"This is a embedded service of a coffee machine.\",\"version\":\"0.1\",\"definitions\":{\"positiveNumber\":{\"minimum\":0.0,\"type\":\"number\"},\"Product\":{\"required\":[\"id\",\"name\"],\"type\":\"object\",\"description\":\"The product of current coffee machine.\",\"properties\":{\"price\":{\"$ref\":\"#/definitions/Price\"},\"id\":{\"$ref\":\"#/definitions/positiveInteger\"},\"image\":{\"type\":\"string\",\"description\":\"Base64 encoded image of a product\"},\"name\":{\"type\":\"string\"}}},\"Price\":{\"required\":[\"amount\",\"currency\"],\"type\":\"object\",\"properties\":{\"currency\":{\"enum\":[\"EUR\",\"USD\",\"LTL\",\"LVL\",\"RUB\",\"SEK\"]},\"amount\":{\"$ref\":\"#/definitions/positiveNumber\"}}},\"positiveInteger\":{\"minimum\":0,\"type\":\"integer\"}},\"$schema\":\"http://json-schema.org/draft-04/schema#\",\"id\":\"http://ttu.ee/denis_konstantinov/embedded_service.schema\"}";




//"\"divide": {
//   "description":"Divide one number by another",
//   "type":"method",
//   "returns":"number",
//   "params":[{"type":"number","name":"dividend","required":true},
//             {"type":"number","name":"divisor","required":true}]
//   },
// "sqrt": {
//   "description":"Find the square root of a number",
//   "type":"method",
//   "returns":"number",
//   "params":[{"type":"number","name":"square","required":true,
//              "minimum":0,"description":"Number to find the square root of"}]
//   },
// "sqrtComplex": {
//   "description":"Find the square root of a number including negative numbers",
//   "type":"method",
//   "returns":["number", {"realPart":{"type":"number"},
//                        "imaginaryPart":{"type":"number"}}],
//   "params":[{"type":"number","name":"square","required":true,
//              "description":"Number to find the square root of"}]
//   },
// "sum": {
//   "description":"Find the sum of the parameters",
//   "type":"method",
//   "returns":"number",
//   "params":[{"type":"number","name":"*",
//              "description":"Number to include in the sum"}]
//   }



