/*
 * JSON Schema primitive types
 * null, boolean, object, array, number, string
 *
 * Keywords
 *
 * OBJECTS
 * '$schema' :
 * '$id' : 
 * 'title' : title of the schema
 * 'description' : a brief description of the schema
 * 'type' : "object" or "boolean"
 * 'properties' : an object representing members of an object
 * 'required' : an array of property keys that are required to have values
 *
 * '$ref': url to a referenced JSON schema
 *
 * 'exclusiveMinimum' : must be greater than this value
 * 'minimum': must be greater than or equal to this value
 * 'maximum': must be less than or equal to this value
 *
 * 'items': describe an array schema
 * 'minItems': minimum number of items in an array
 * 'uniqueItems': all items in array must be unique
 */

/*json schema formats
 * https://json-schema.org/understanding-json-schema/reference/string.html
 * date-time
 * time
 * date
 * email
 * idn-email
 * hostname
 * idn-hostname
 * ipv4
 * ipv6
 * uri
 * uri-reference
 * iri
 * iri-reference
 * uri-template
 * json-pointer
 * relative-json-pointer
 * regex
 */

/* "$id" : "https://example.com/person.schema.json"
 * "$schema" : "http://json-schema.org/draft-07/schema#"
 * "title" : "MyInstanceDataModel"
 * "type" : "object"
 * "properties": {}
*/



$id
$schema
$ref
$comment
title
description
default
readOnly
examples
multipleOf
maximum
exclusiveMaximum
minimum
exclusiveMinimum
maxLength
minLength
pattern
maxItems
minItems
uniqueItems
contains
maxProperties
minProperties
required
additionalProperties
definitions
properties
patternProperties
dependencies
type


types
 array
 boolean
 integer
 null
 number
 object
 string

Numeric
 multipleOf
 maximum
 minimum
 exclusiveMinimum

Strings
 maxLength
 minLength
 pattern

Arrays
 items
 additionalItems
 maxItems
 minItems
 uniqueItems
 contains

Objects
 maxProperties
 minProperties
 required
 properties
 patterProperties
 additionalProperties
 dependencies
 propertyNames
 
