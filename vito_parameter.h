
struct vito_parameter {
  char name[50];
  char description[300];
  void (*readValue)(char *valuestring);
  void (*writeValue)(char *valuestring);
  char unit[10];
};

