
struct vito_parameter {
  char name[50];
  char unit[6];
  int address;
  int min;
  int max;
  int (*readValue)(struct vito_parameter *self, char *valuestring);
  int (*writeValue)(struct vito_parameter *self, char *valuestring);
};

