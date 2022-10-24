// Output the contents of an objectStruct
//  into an obj file.
void write_obj_file (objectStruct object_data, char* filename);

// Create a new objectStruct to contain the
//  decimated model.
// This one will not include the vertices
//  and faces removed during the process.
objectStruct get_decimated_object (objectStruct complex_object);
