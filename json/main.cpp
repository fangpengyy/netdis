#include <stdio.h>
#include <jansson.h>

 
int event_jansson()
{
    json_t *objectmsg;
    char *result;
 
    objectmsg = json_object();
 
    json_object_set_new (objectmsg, "inc", json_string("value-incID"));
    json_object_set_new (objectmsg, "src", json_string("a"));
    json_object_set_new (objectmsg, "des", json_string("b"));
    json_object_set_new (objectmsg, "protocol", json_string("c"));
    json_object_set_new (objectmsg, "policy", json_string("d"));
    json_object_set_new (objectmsg, "snapshot", json_string("e"));
    json_object_set_new (objectmsg, "name", json_string("f"));
    json_object_set_new (objectmsg, "Type", json_string("g"));
    json_object_set_new (objectmsg, "Path", json_string("h"));
    json_object_set_new (objectmsg, "domain", json_string("i"));
 
    result = json_dumps(objectmsg, JSON_PRESERVE_ORDER);
 
    printf("result=%s\n",result);
 
    free(result);
    json_decref(objectmsg);

return 0;
}
 
int main()
{
  event_jansson();
  return 0;
}
