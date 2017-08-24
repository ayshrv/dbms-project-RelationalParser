/*****************************************
*    	Relational Algebra Parser        *
*    	Author: Ayush Shrivastava        *
*    	Roll No.: 14075014               *
*    	B.Tech CSE 3rd Year              *
*										 *
******************************************/

//Including All the necessary header files
#include <iostream>
#include <sstream>
#include <fstream>
#include <climits>
#include <cctype>
#include <iomanip>
#include <string.h>
#include <vector>
#include <stack>
#include <map>
#include <set>
#include <algorithm>

using namespace std;

//Constants For Displaying in Tabular Form In Terminal
const char separator    = ' ';
const int nameWidth     = 28;
const int numWidth      = 10;

//Defining possible datatypes
enum Type
{
    INT,DOUBLE,CHAR,STRING,BOOL
};

//Converts a number or any datatype to string
//Input: Variable Number of any data type
//Output: String containing that value
template <typename T>
string numberToString ( T Number )
{
	ostringstream ss;
	ss << Number;
	return ss.str();
}

// trim from start
//Input: string to be trimmed
//Output: left trimmed string
static inline std::string ltrim(std::string s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
//Input: string to be trimmed
//Output: right trimmed string
static inline std::string rtrim(std::string s) 
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
//Input: string to be trimmed
//Output: returns string trimmed from both sides
static inline std::string trim(std::string s) 
{
    return ltrim(rtrim(s));
}

//Converts a string to Integer while checking for possible errors
int stringToInt(string v)
{
	if(v=="")
	{
		string error ="TypeChecking Error: No input given in place of INT";
		throw error;
	}
	//check for negative sign
	bool negate = (v[0]=='-');
	int sign=0;
	if(v[0]=='-' || v[0]=='+')
	{
		sign++;
		if(v.length()==1)
		{
			string error ="TypeChecking Error: Only sign character provided";
			throw error;
		}
	}

	int result=0;
	for(int i=sign;i<v.length();i++)
	{
		if(v[i]>='0' && v[i]<='9')
			result = result*10 - (v[i] - '0');
		else
		{
			string error ="TypeChecking Error: Wrong input given in place of INT";
			throw error;
		}
	}
	//Comparing for overflow
	string int_max=numberToString(INT_MAX);
	string s;
	int maxlength = int_max.length();

	if(sign==1)
	{
		s=v.substr(1,v.length()-1);
	}
	else
		s=v;

	int vlength = s.length();
	for(int i=0;i<(vlength-maxlength);i++)
		int_max= "0" + int_max;

	for(int i=0;i<(maxlength-vlength);i++)
		s="0"+s;

	if(s>=int_max)
	{
		string error ="Overflow Error: INT value should be between -"+numberToString(INT_MAX)+" to +"+numberToString(INT_MAX);
		throw error;
	}

	return negate ? result : -result;
}

//Converts a string to Double while checking for possible errors
double stringToDouble(string v)
{
	if(v=="")
	{
		string error="TypeChecking Error: No input given in place of DOUBLE";
		throw error;
	}
	if(v[0]=='+' || v[0]=='-')
	{
		if(v.length()==1)
		{
			string error="TypeChecking Error: Only sign character provided in place of DOUBLE";
			throw error;
		}
	}
	int sign=0;
	int decimal=0;
	for(int i=0;i<v.length();i++)
	{
		if(v[i]=='+' || v[i]=='-')
		{
			if(sign==1)
			{
				string error="TypeChecking Error: Extra Sign added in input for DOUBLE";
				throw error;
			}
			else
				sign=1;
		}
		else if(v[i]=='.')
		{
			if(decimal==1)
			{
				string error="TypeChecking Error: Extra Decimal added in input for DOUBLE";
				throw error;
			}
			else
				decimal=1;
		}
		else if(v[i]>='0' && v[i]<='9');
		else
		{
			string error="TypeChecking Error: Wrong input given in place of DOUBLE";
			throw error;
		}
	}

	return atof(v.c_str());
}
//Converts a string to char if its length is 1
char stringToChar(string v)
{
	if(v=="")
	{
		string error="TypeChecking Error: No input given in place of CHAR";
		throw error;
	}
	else if(v.length()!=1)
	{
		string error="TypeChecking Error: More than one characters given in place of CHAR";
		throw error;
	}
	return v[0];
}

//Converts a string to bool
bool stringToBool(string v)
{
	std::transform(v.begin(), v.end(), v.begin(), ::toupper);
	if(v=="TRUE" || v=="T")
		return true;
	if(v=="FALSE" || v=="F")
		return false;
	string error="TypeChecking Error: Wrong input given in place of BOOL";
	throw error;
}

//Checks whether a string contains only alphabets or not
bool isAlpha(string name)
{
	for(int i=0;i<name.size();i++)
		if(!( (name[i]>='a' && name[i]<='z')|| (name[i]>='A' && name[i]<='Z') ) )
			return false;
	return true;
}
//Checks whether a string contains only alphabets & numerals or not
bool isAlphaNumeric(string name)
{
	for(int i=0;i<name.size();i++)
		if(!( (name[i]>='a' && name[i]<='z')|| (name[i]>='A' && name[i]<='Z') || (name[i]>='0' && name[i]<='9') || name[i]=='_' || name[i]==' ' ||name[i]=='-'|| name[i]=='|') )
			return false;
	return true;
}

//Function for printing elements on terminal
template<typename T> void printElement(T t, const int& width,int flag)
{
	if(flag==1)
	    std::cout << std::left << std::setw(width) << std::setfill(separator) << t;
	else
	    std::cout << std::setw(width) << std::setfill(separator) << t;
}

//Generic function used to throw error from anywhere in the code
void throwError(string error)
{
	throw error;
}

//Attribute whihc defines the schema of a table
class attribute
{

private:
	string attributeName;  //Name of attribute
    Type dataType;			//Its DataType

    // Converting ENUM to String
	string enumToString(Type v)
   	{
   		switch(v)
   		{
   			case 0:
   				return "INT";
   				break;
   			case 1: 
   				return "DOUBLE";
   				break;
   			case 2:
   				return "CHAR";
   				break;
   			case 3: 
   				return "STRING";
   				break;
   			case 4:
   				return "BOOL";
   				break;
   		}
   	}

   	//Converting String To Enum
   	Type stringToEnum(string v)
   	{
   		std::transform(v.begin(), v.end(), v.begin(), ::toupper);
		if(v=="INT")
			return INT;
		else if(v=="DOUBLE")
			return DOUBLE;
		else if(v=="CHAR")
			return CHAR;
		else if(v=="STRING")
			return STRING;
		else if(v=="BOOL")
			return BOOL;
		else
		{
			string error = "DataType Should Be Only Among INT,DOUBLE,CHAR,STRING,BOOL";
			throw error;
		}
   	}

public:
    attribute() {}															//Declare an empty attribute
    attribute(string attributeName,Type dataType)							//Declare an attribute with attributeName as name and dataType as its type
	{																		//Check for attribute name to be alphabetical
		if(!isAlpha(attributeName))
		{											
			string error = "Attribute Name should have alphabets only";
			throw error;
		}
	    this->attributeName = attributeName;
	    this->dataType = dataType;
	}
	attribute(string attributeName,string dataType)
	{
	    setValues(attributeName,dataType);
	}

	bool operator==(const attribute &x) const {								//Comparator for attribute object

		if(attributeName==x.attributeName)
		{
			return true;
		}
		return false;
    }
    //Set values of a attribute after declaring it
	void setValues(string attributeName, string dataType)				
	{
		if(!isAlpha(attributeName))
		{
			string error = "Attribute Name should have alphabets only";
			throw error;
		}
		this -> attributeName = attributeName;
		this -> dataType = stringToEnum(dataType);
		
	}
	//returns the name of the attribute
	string getName()													
	{
		return attributeName;
	}
	// returns the datatype of attribute 
	Type getDataType()
	{
		return dataType;
	}
	//returns the datatype in string
	string getDataTypeString()
	{
		return enumToString(getDataType());
	}
	//Used for printing the attribute name on the screen
	void printAttribute()
	{
		if(getDataType()==STRING)
			printElement(getName(),nameWidth,1);
		else
			printElement(getName(),numWidth,0);
		std::cout<<" ";
	}
	//Used for printing the attribute type on the screen
	void printDatatype()
	{
		switch(getDataType())
		{
			case INT:
				printElement(getDataTypeString(),numWidth,0);
				break;
			case DOUBLE:
				printElement(getDataTypeString(),numWidth,0);
				break;
			case BOOL:
				printElement(getDataTypeString(),numWidth,0);
				break;
			case CHAR:
				printElement(getDataTypeString(),numWidth,0);
				break;
			case STRING:
				printElement(getDataTypeString(),nameWidth,1);
				break;
		}
		std::cout<<" ";
	}

};
//union of Int,Double,Bool,Char,String for storing value of an element of record
union dataCell
{
    int valueInt;
    double valueDouble;
    bool valueBool;
    char valueChar;
    char valueString[500];													//Maximum length of allowed string is 500.
};
//Wrapper class for the union dataCell. Also tells us about the datatype of the dataCell.
class cell
{
private:
    dataCell cellValue;														//Union which stores the value of cell
    Type dataType;															//Specifies which cell contains what
    bool null;																//To check whether an element is null or not

   	string enumToString(Type v)
   	{
   		switch(v)
   		{
   			case 0:
   				return "INT";
   				break;
   			case 1: 
   				return "DOUBLE";
   				break;
   			case 2:
   				return "CHAR";
   				break;
   			case 3: 
   				return "STRING";
   				break;
   			case 4:
   				return "BOOL";
   				break;
   		}
   	}

   	Type stringToEnum(string v)
   	{
   		std::transform(v.begin(), v.end(), v.begin(), ::toupper);
		if(v=="INT")
			return INT;
		else if(v=="DOUBLE")
			return DOUBLE;
		else if(v=="CHAR")
			return CHAR;
		else if(v=="STRING")
			return STRING;
		else if(v=="BOOL")
			return BOOL;
   	}
	
public:
    cell()																//Empty cell means NULL Value
	{
	    null=true;
	}
    cell(int v)															//Set values of the attribute as INT after declaring it
	{
	    null=false;
	    cellValue.valueInt=v;
	    dataType=INT;
	}
    cell(double v)														//Set values of the attribute as DOUBLE after declaring it
	{
	    null=false;
	    cellValue.valueDouble=v;
	    dataType=DOUBLE;
	}
	cell(bool v)														//Set values of a attribute as BOOL after declaring it
	{
		null=false;
		cellValue.valueBool=v;
		dataType=BOOL;
	}
    cell(char v)														//Set values of a attribute as CHAR after declaring it
	{
	    null=false;
	    cellValue.valueChar=v;
	    dataType=CHAR;
	}
    cell(string v)														//Set values of a attribute as STRING after declaring it
	{
		if(v.length()>=500)												// Value of string should not be more than 500 
		{
			string error="String Overflow Exception: String Size cannot be greater than 500";
			throw error;
		}

	    null=false;
	    for(int i=0;i<v.length();i++)									//saving the value of string as a C string
	        cellValue.valueString[i]=v[i];
	    cellValue.valueString[v.length()]='\0';
	    dataType=STRING;
	}

	bool operator<(const cell &x) const {

		if(x.dataType!=dataType)										//Comparator for sorting them in Map
		{
			return true;
		}
		switch(dataType)
		{
			case INT:
				return cellValue.valueInt<x.cellValue.valueInt;
				break;
			case DOUBLE:
				return cellValue.valueDouble<x.cellValue.valueDouble;
				break;
			case BOOL:
				return cellValue.valueBool<x.cellValue.valueBool;
				break;
			case CHAR:
				return cellValue.valueChar<x.cellValue.valueChar;
				break;
			case STRING:
				return numberToString(cellValue.valueString)<numberToString(x.cellValue.valueString);
				break;
		}
    }

	void setValueInt(int v)												//Overrides the value of the cell to be INT
	{
		null=false;
	    cellValue.valueInt=v;
	    dataType=INT;
	}
	void setValueDouble(double v)										//Overrides the value of the cell to be DOUBLE
	{
	    null=false;
	    cellValue.valueDouble=v;
	    dataType=DOUBLE;
	}
	void setValueBool(bool v)											//Overrides the value of the cell to be BOOL
	{
		null=false;
		cellValue.valueBool=v;
		dataType=BOOL;
	}
	void setValueChar(char v)											//Overrides the value of the cell to be CHAR
	{
		null=false;
	    cellValue.valueChar=v;
	    dataType=CHAR;
	}
	void setValueString(string v)										//Overrides the value of the cell to be STRING
	{
		if(v.length()>=500)
		{																//Again a check for 500 characters
			string error= "String Overflow Exception: String Size cannot be greater than 500";
			throw error;
		}

		null=false;
	    for(int i=0;i<v.length();i++)
	        cellValue.valueString[i]=v[i];
	    cellValue.valueString[v.length()]='\0';
	    dataType=STRING;
	}
	// Returns True if Cell is NULL
	bool isNull()
	{
		if(null==true)
			return true;
		return false;
	}
	//returns the data type of cell
	Type getDataType()
	{
		return dataType;
	}
	//returns the data type of cell in string
	string getDataTypeString()
	{
		return enumToString(getDataType());
	}
	//check whether datatype of cell is INT and returns the INT value
	int getInt()
	{
		if(isNull())
		{
			string error= "Null Value Exception: No Value Stored in Cell";
			throw error;
		}
		else if(getDataType()!=INT)
		{
			string error= "Wrong DataType Exception: INT Value is not stored in Cell";
			throw error;
		}

		return cellValue.valueInt;
	}
	//check whether datatype of cell is Double and returns the Double value
	double getDouble()
	{
		if(isNull())
		{
			string error= "Null Value Exception: No Value Stored in Cell";
			throw error;
		}
		else if(getDataType()!=DOUBLE)
		{
			string error= "Wrong DataType Exception: DOUBLE Value is not stored in Cell";
			throw error;
		}
		
		return cellValue.valueDouble;
	}
	//checks whether datatype of cell is Bool ands returns  the Bool value
	int getBool()
	{
		if(isNull())
		{
			string error= "Null Value Exception: No Value Stored in Cell";
			throw error;
		}
		else if(getDataType()!=BOOL)
		{
			string error= "Wrong DataType Exception: BOOL Value is not stored in Cell";
			throw error;
		}

		return cellValue.valueBool;
	}
	//returns Bool type in string
	string getBoolString()
	{
		return getBool()?"True":"False";
	}

	//Returns the value of Datacell in Char while checking error for datatype mismatch
	char getChar()
	{
		if(isNull())
		{
			string error= "Null Value Exception: No Value Stored in Cell";
			throw error;
		}
		else if(getDataType()!=CHAR)
		{
			string error= "Wrong DataType Exception: CHAR Value is not stored in Cell";
			throw error;
		}
		
		return cellValue.valueChar;
	}

	//Returns the value of string in Char while checking error for datatype mismatch
	string getString()
	{
		if(isNull())
		{
			string error= "Null Value Exception: No Value Stored in Cell";
			throw error;
		}
		else if(getDataType()!=STRING)
		{
			string error= "Wrong DataType Exception: STRING Value is not stored in Cell";
			throw error;
		}
		
		return string(cellValue.valueString);
	}

	//returns the value of DataUnion in string 
	//i.e. if its value is in int, then the int will be represented in the form of string 
	string getValueString()
	{
		if(isNull())
		{
			string error= "Null Value Exception: No Value Stored in Cell";
			throw error;
		}
		else
		{
			string s;
			switch(getDataType())
			{
				case INT:
					s=numberToString(getInt());
					break;
				case DOUBLE:
					s=numberToString(getDouble());
					break;
				case BOOL:
					s=numberToString(getBool());
					break;
				case CHAR:
					s=numberToString(getChar());
					break;
				case STRING:
					s=getString();
					break;
			}
			return s;
		}
	}

	//Prunts the value of the cell on the terminal
	void printCell()
	{
		switch(getDataType())
		{
			case INT:
				printElement(getInt(),numWidth,0);
				break;
			case DOUBLE:
				printElement(getDouble(),numWidth,0);
				break;
			case BOOL:
				printElement(getBoolString(),numWidth,0);
				break;
			case CHAR:
				printElement(getChar(),numWidth,0);
				break;
			case STRING:
				printElement(getString(),nameWidth,1);
				break;
		}
		std::cout<<" ";
	}

	//Prints the datatype of the cell on the terminal
	void printDatatype()
	{
		switch(getDataType())
		{
			case INT:
				printElement(getDataTypeString(),numWidth,0);
				break;
			case DOUBLE:
				printElement(getDataTypeString(),numWidth,0);
				break;
			case BOOL:
				printElement(getDataTypeString(),numWidth,0);
				break;
			case CHAR:
				printElement(getDataTypeString(),numWidth,0);
				break;
			case STRING:
				printElement(getDataTypeString(),nameWidth,1);
				break;
		}
		std::cout<<" ";
	}

};

//Table class is the basic entity which represents the relation in Relational Algebra
//A Table consists of all the basic properties that it should.
//tableName, Schema, noOfAttributes, noOfRecords, dataInTable etc.
class Table
{
private:
	//These datatypes are set to private so that no one can access them without the use of public methods provided for them.
	std::string tableName;                              	//Defines the name of table
	int noOfAttributes;										//No. Of Attributes in the Table initially zero (NULL TABLE)
	int noOfRecords;										//No. Of Records in the Table initially zero (EMPTY TABLE)
	std::map<std::string,int> attributeIndexMap;			//Defines a mapping between the name of the attribute and their index in the schema and dataInTable
	std::vector<attribute> schema;							//Schema is defined as vector of class objects attributes which are defined above which contains attributeName and attributeType

public:
	//Public Datatypes and methods to be accessed by other Classes.
	std::set<std::vector<cell> > dataInTable;				//Stores the Tuples in dataInTable. Each Tuple will be vector<cell>, and it will be stored in set<vector<cell>>
															//dataInTable is implemented as a Set, so no repeating tuples will be allowed.

	//Constructor For NULL TABLE
	Table()
	{
		noOfAttributes=0;
		noOfRecords=0;
	}
	//Constructor which creates a Table as:
	//		Table : tableName
	//		Schema:	attributeNames[0]   attributeTypes[0]
	//				attributeNames[1]   attributeTypes[1]
	//				attributeNames[2]   attributeTypes[2]
	// and So on....
	Table(std::string tableName, int noOfAttributes, vector<string> attributeNames, vector<string> attributeTypes)
	{
		if(!isAlphaNumeric(tableName))
		{
			string error = "Table Name should be alphanumeric only";
			throw error;
		}
		if(tableName=="")
		{
			string error = "Table Name should not be Empty";
			throw error;
		}
		this->tableName = tableName;
		setSchema(noOfAttributes, attributeNames, attributeTypes);

	}

	// return the table name
	string getTableName()
	{
		return tableName;
	}
	//Sets the name of table and checks that table name is alphanumeric only.
	void setTableName(string name)
	{
		if(!isAlphaNumeric(name))
		{
			string error = "Table Name should be alphanumeric only";
			throw error;
		}

		this->tableName = name;
	}
	//Returns the no of attributes in the table.
	int getNoOfAttributes()
	{
		return noOfAttributes;
	}
	//Sets the no of attributes of the table
	void setNoOfAttributes(int number)
	{
		this->noOfAttributes=number;
	}

	//Returns no of data Tuples stored in the table presently.
	int getNoOfRecords()
	{
		return noOfRecords;
	}

	// Returns The schema of the table. Methos to access Private Data
	std::vector<attribute> getSchema()
	{
		return schema;
	}

	//Sets the schema of table to the given values.
	//Input: No. of Attributes in the table
	//       Names of Attributes in Vector
	// 		 Types of Attributes in Vector
	void setSchema(int noOfAttributes, vector<string> attributeNames, vector<string> attributeTypes)
	{
		if(noOfAttributes==0)													// Checks that Attributes should not be NULL
		{
			string error= "NULL Schema Cannot be Allowed In "+getTableName();
			throw error;
		}							
		this->noOfAttributes = noOfAttributes;
		attributeIndexMap.clear();					
		dataInTable.clear();
		noOfRecords=0;
		schema = std::vector<attribute>(noOfAttributes);
		for(int i=0;i<noOfAttributes;i++)
		{
			if(attributeExitsinTable(attributeNames[i]))							//If Attribute Already Exists, then throw an error
			{
				string error= "Corrupt Data Exception: Duplicate Attributes Are Not Allowed In "+getTableName();
				throw error;
			}																	
			schema[i].setValues(attributeNames[i],attributeTypes[i]);				//Sets the values of ith attribute
			attributeIndexMap[schema[i].getName()]=i;								//Saves its corresponding index
		}
	}

	//Rename the schema to a new schema
	//i.e. Rename the attributes to new attribute names
	void renameSchema(vector<string> newAttributes)
	{
		if(newAttributes.size()!=noOfAttributes)					//New attributes list provided should be equal to No. of attributes in schema
		{
			string error= "Corrupt Data Exception: No. of attributes in Rename Function should be equal to no. of attributes in "+getTableName();
			throw error;											// Throws an error
		}
		attributeIndexMap.clear();									//Clears the Attribute Index Map
		std::vector<attribute> newSchema(noOfAttributes);			//Declare the new schema
		for(int i=0;i<noOfAttributes;i++)
		{
			if(attributeExitsinTable(newAttributes[i]))				//Check for duplicate attributes
			{
				string error= "Corrupt Data Exception: Duplicate Attributes Are Not Allowed In "+getTableName();
				throw error;
			}
			newSchema[i].setValues(newAttributes[i],schema[i].getDataTypeString());
			attributeIndexMap[newAttributes[i]]=i;
		}

		schema = newSchema;											// Overwrite the previous schema


	}

	//Prints the schema in full format, for example,
	//Table : Teacher
	//1. Name STRING
	//2. Age INT
	//3. Sex CHAR
	// and so on
	void showSchema()
	{
		cout<<"Table: "<<tableName<<endl;
		for(int i=0;i<schema.size();i++)
			cout<<i+1<<". "<<schema[i].getName()<<" "<<schema[i].getDataTypeString()<<std::endl;
	}

	//Prints the schema in two lines, for example,
	//Table: Teacher
	//Name 				Age 		Sex
	//STRING            INT        CHAR
	void showSchemaHeader()
	{

		for(int i=0;i<noOfAttributes;i++)
		{
			schema[i].printAttribute();
		}
		std::cout<<std::endl;

		for(int i=0;i<noOfAttributes;i++)
		{
			schema[i].printDatatype();
		}
		std::cout<<std::endl;
	}

	//Prints the data of table on the terminal
	//Table: Teacher
	//Name 				Age 		Sex
	//STRING            INT        CHAR
	//Ayush              18         M
	//Manisha              38         F
	void showData()
	{
		cout<<"Table: "<<tableName<<endl;
		showSchemaHeader();
		std::set<std::vector<cell> >::iterator it;
		for(it=dataInTable.begin(); it!=dataInTable.end(); it++)
		{
			vector<cell> Tuple = *it;
			for(int j=0;j<noOfAttributes;j++)
				Tuple[j].printCell();

			std::cout<<std::endl;
		}
			std::cout<<std::endl;
	}

	// Inserts the dataTuple in the Table
	void insertIntoTable(std::vector<std::string>& values)
	{
		if(values.size()!=noOfAttributes)				//Check if DataTuple contains all the attributes of the schema
		{
			string error= "Corrupt Data Exception: No. of entries in tuple should be equal to no. of attributes";
			throw error;
		}
		std::vector<cell> v(noOfAttributes);
		for(int i=0;i<noOfAttributes;i++)
		{
			switch(schema[i].getDataType())				// Check the datatype of cell and coresspondingly insert data.
			{
				case INT:
					v[i].setValueInt(stringToInt(values[i]));
					break;
				case DOUBLE:
					v[i].setValueDouble(stringToDouble(values[i]));
					break;
				case BOOL:
					v[i].setValueBool(stringToBool(values[i]));
					break;
				case CHAR:
					v[i].setValueChar(stringToChar(values[i]));
					break;
				case STRING:
					v[i].setValueString(values[i]);
					break;
			}
		}
		if(!dataTupleExists(v))                          // If data provided not exists in table, then insert it.
			addDataTuple(v);
		else
		{												// Else, Duplicate data error.
			string error= "Corrupt Data Exception: Duplicate Data Tuple Entered in Table: "+getTableName();
			throw error;
		}
	}

	//Check if attribute exists in Table or not
	bool attributeExitsinTable(string attribute)
	{
		if(attributeIndexMap.count(attribute)>0)
			return true;
		return false;
	}

	//Returns the attribute object
	//Input: Attribute Name
	// Output: Attribute Object
	attribute getAttributeByName(string name)
	{
		if(!attributeExitsinTable(name))
		{
			string error="Attribute Match Error: No attribute \""+name+"\" found in Table "+getTableName();
			throw error;
		}

		int index = attributeIndexMap[name];
		return schema[index];
	}

	//Returns the attribute index in the schema
	// Input: Attribute Name
	// Output: Index of that attribute if it exists
	int getAttributeIndexByName(string name)
	{
		if(!attributeExitsinTable(name))
		{
			string error="Attribute Match Error: No attribute \""+name+"\" found in Table "+getTableName();
			throw error;
		}

		return attributeIndexMap[name];
	}

	// Adds Attribute to schema if it does not exists already
	void addAttributeToSchema(attribute newAttribute)
	{
		if(attributeExitsinTable(newAttribute.getName()))
		{
			string error= "Corrupt Data Exception: Duplicate Attributes Are Not Allowed In "+getTableName();
			throw error;
		}
		schema.push_back(newAttribute);
		attributeIndexMap[newAttribute.getName()]=noOfAttributes;
		noOfAttributes++;

	}

	// Adds a datatuple in dataInTable
	void addDataTuple(vector<cell> dataTuple)
	{
		dataInTable.insert(dataTuple);
		noOfRecords++;
	}

	// Check if Tuple exists in the table or not
	bool dataTupleExists(vector<cell> dataTuple)
	{
		if(dataInTable.find(dataTuple) != dataInTable.end())
			return true;
		return false;
	}

	//Checks if Schema of the underlying table is disjoint with given table
	// Input: Schema of Other Table
	//Output: Disjoint or Not in BOOL
	bool isDisjointSchema(std::vector<attribute> otherSchema)
	{
		for(int i=0;i<otherSchema.size();i++)
		{
			for(int j=0;j<schema.size();j++)
			{
				if(otherSchema[i]==schema[j])
					return false;
			}
		}
		return true;
	}

	//Checks if schema of two tables is exactly same i.e., their attribute names and attribute types should be same in the given order
	// Input: Table A
	// Output: True/False
	bool isSchemaSame(Table A)
	{
		int flag;
		std::vector<attribute> schemaA = A.getSchema();
		for(int i=0;i<schema.size();i++)
		{
			flag=0;
			for(int j=0;j<schemaA.size();j++)
			{
				if(schema[i].getName()==schemaA[j].getName() && schema[i].getDataType()==schemaA[j].getDataType())
				{
					flag=1;
					break;
				}
			}
			if(flag==0)
					return false;
		}

		for(int j=0;j<schemaA.size();j++)
		{
			flag=0;
			for(int i=0;i<schema.size();i++)
			{
				if(schema[i].getName()==schemaA[j].getName() && schema[i].getDataType()==schemaA[j].getDataType())
				{
					flag=1;
					break;
				}
			}
			if(flag==0)
					return false;
		}

		return true;
	}

};

//adds two data tuples and return a new tuple
//Input: DataTuple A, DataTuple B
//Output: X which is concatenation of two datatuples A and B.
vector<cell> addTwoTuples(vector<cell> A, vector<cell> B)
{
	vector<cell> X;
	for(int i=0;i<A.size();i++)
	{
		X.push_back(A[i]);
	}
	for(int i=0;i<B.size();i++)
	{
		X.push_back(B[i]);
	}
	return X;
}

// Returns a vector of strings which were seperated by delimiter in the given string
//Input: "A,B,C,D,EF" and delimiter as ','
// Output: Vector: A
//				   B
//				   C
//  		 	   D
// 				   EF
vector<string> commaSeparatedStrings(string list ,char delimiter)
{
    stringstream ss(list);
    vector<string> result;

    while(ss.good())
    {
        string substr;
        getline(ss,substr,delimiter);
        result.push_back(substr);
    }
    return result;
}

//Function for Project Operation
// Input: A Table table and vector of strings containting names of the projected attributes.
//Output: Projected Table output
Table ProjectTable(Table table, vector<string> attributes)
{
	if(attributes.size()==0)															// Check for zero projected attributes
	{
		string error="Attribute Match Error: No attributes are provided for Project Function";
		throw error;
	}
	Table output;
	output.setTableName(table.getTableName());											//set the table name of previous table
	output.dataInTable.clear();
	for(int i=0;i<attributes.size();i++)
	{
		output.addAttributeToSchema(table.getAttributeByName(attributes[i]));			//Set schema for projected table
	}
	std::set<std::vector<cell> >::iterator it;
	for (it=table.dataInTable.begin(); it!=table.dataInTable.end(); it++)
	{
		vector<cell> Tuple = *it;
		std::vector<cell> v(output.getNoOfAttributes());
		for(int j=0;j<attributes.size();j++)
		{
			int index = table.getAttributeIndexByName(attributes[j]);					//Get projected attribute values in the data Tuple
			v[j]=Tuple[index];

		}
		if(!output.dataTupleExists(v))													//Check if Projected value is not present, then add it
			output.addDataTuple(v);														// This is a check for Duplicate projected values.
	}

	return output;
}

//Function for Rename Operation
//Input: A Table table and renamed tablename tableName
//Output: Table object with renamed table name
Table RenameTable(Table table, string tableName)
{
	Table output = table;
	output.setTableName(tableName);
	return output;
}

//Function for Rename Table as well as Rename Attributes
// Input: A Table table, a String tableName which is new table name and vector of Strings defining names of new attributes
Table RenameTable(Table table, string tableName, vector<string> attributes)
{
	Table output = table;
	output.renameSchema(attributes);              				//Renames the Schema of New Table
	output.setTableName(tableName);
	return output;
}

//Function for Cartesian Product Operation
// Input: Table A, Table B
// Output: A x B
Table CartesianProduct(Table A, Table B)
{
	std::vector<attribute> schemaB = B.getSchema();
	if(!A.isDisjointSchema(schemaB))											// Check for Disjoint Schema
	{
		string error="Cartesian Product Error: Table \""+A.getTableName()+"\" and Table \""+B.getTableName()+"\" should have completely disjoint schema";
		throw error;
	}
	Table output = A;
	output.setTableName("|"+A.getTableName()+" x "+B.getTableName()+"|");			//sets the name as | A x B |
	output.dataInTable.clear();
	std::vector<attribute>::iterator it;
	for(it = schemaB.begin(); it != schemaB.end(); it++)
	{
		output.addAttributeToSchema(*it);										//add attributes to new table formed
	}
	std::set<std::vector<cell> >::iterator it_A;
	std::set<std::vector<cell> >::iterator it_B;
	for(it_A=A.dataInTable.begin(); it_A!=A.dataInTable.end(); it_A++)
	{
		for(it_B=B.dataInTable.begin(); it_B!=B.dataInTable.end(); it_B++)
		{
			output.addDataTuple(addTwoTuples(*it_A,*it_B));						//add cross producted datatuple to new table 
		}
	}
	return output;
}

//Function for Union Operation
// Input: Table A , Table B
// Output: A U B
Table UnionTables(Table A, Table B)
{
	if(!A.isSchemaSame(B))													//Schema should be same for both of them
	{
		string error="Union Table Error: Table \""+A.getTableName()+"\" and Table \""+B.getTableName()+"\" do not have same schemas";
		throw error;
	}

	Table output = A;
	output.setTableName("|"+A.getTableName()+" Union "+B.getTableName()+"|");				//New Table Name Set as  | A Union B |
	std::set<std::vector<cell> >::iterator it;
	for(it = B.dataInTable.begin(); it != B.dataInTable.end(); it++)
	{
		if(!A.dataTupleExists(*it))
		{
			output.addDataTuple(*it);											//Add datatuples to new table aafter checking for duplicate entries
		}
	}

	return output;
}

//Function for Set Difference Operation
// Input: Table A , Table B
// Output: A - B
Table SetDifferenceTable(Table A, Table B)
{
	if(!A.isSchemaSame(B))													//Schema should be same for both of them
	{
		string error="SetDifference Table Error: Table \""+A.getTableName()+"\" and Table \""+B.getTableName()+"\" do not have same schemas";
		throw error;
	}

	Table output = A;
	output.setTableName("|"+A.getTableName()+" - "+B.getTableName()+"|");				//New Table Name Set as  | A - B |
	output.dataInTable.clear();
	std::set<std::vector<cell> >::iterator it;
	for(it=A.dataInTable.begin(); it!=A.dataInTable.end(); it++)
	{
		if(!B.dataTupleExists(*it))
			output.addDataTuple(*it);											//Add datatuples of A to new table if they does not exist in B
	}

	return output;

}

//SELECT Parser for handling predicate in SELECT Query
class SelectParser
{
private:
	//Types defined for QueueObjects
	enum DataType
	{
		Integer, Double, Char, Bool, String, Variable, Operator, Bracket
	};
	//Data wrapper for all types
	union QueueUnion
	{
	    int valueInt;
	    double valueDouble;
	    char valueChar;
	    bool valueBool;
	    char valueString[500];
	    char Op[3];
	};

	//class Queueobject will represent any element i.e. it can be a variable, an integer, double, string , char
	class QueueObject
	{
	private:
		QueueUnion Data;
		DataType Type;
	public:
		QueueObject() { }
		QueueObject(cell data)
		{
			switch(data.getDataType())
			{
				case INT:
					setInt(data.getInt());
					break;
				case DOUBLE:
					setDouble(data.getDouble());
					break;
				case BOOL:
					setBool(data.getBool());
					break;
				case CHAR:
					setChar(data.getChar());
					break;
				case STRING:
					setString(data.getString());
					break;
			}
		}
		QueueObject(string value, string type)
		{
			if(type=="Integer")
			{
				Data.valueInt=stringToInt(value);
				Type=Integer;
			}
			else if(type=="Double")
			{
				Data.valueDouble=stringToDouble(value);
				Type=Double;
			}
			else if(type=="Char")
			{
				Data.valueChar=stringToChar(value);
				Type=Char;
			}
			else if(type=="Bool")
			{
				Data.valueBool=stringToBool(value);
				Type=Bool;
			}
			else if(type=="String")
			{
				if(value.length()>=500)
				{
					string error="String Overflow Exception: String Size cannot be greater than 500";
					throw error;
				}

				for(int i=0;i<value.length();i++)
			        Data.valueString[i]=value[i];
			    Data.valueString[value.length()]='\0';
				Type=String;
			}
			else if(type=="Variable")
			{
				if(value.length()>=500)
				{
					string error="String Overflow Exception: String Size cannot be greater than 500";
					throw error;
				}

				for(int i=0;i<value.length();i++)
			        Data.valueString[i]=value[i];
			    Data.valueString[value.length()]='\0';
			    Type=Variable;
			}
			else if(type=="Operator")
			{
				if(value.length()==1)
				{
					Data.Op[0]=value[0];
					Data.Op[1]='\0';
				}
				else if(value.length()==2)
				{
					Data.Op[0]=value[0];
					Data.Op[1]=value[1];
					Data.Op[2]='\0';
				}
				else
				{
					string error="Select Operator Exception: Wrong Operator Passed In Select Query";
					throw error;
				}
				Type=Operator;
			}
			else if(type=="Bracket")
			{
				Data.valueChar=value[0];
				Type=Bracket;
			}
		}

		void show()
		{
			switch(getType())
			{
				case Integer:
					cout<<getInt()<<" "<<"Integer"<<endl;
					break;
				case Double:
					cout<<getDouble()<<" "<<"Double"<<endl;
					break;
				case Char:
					cout<<getChar()<<" "<<"Char"<<endl;
					break;
				case String:
					cout<<getString()<<" "<<"String"<<endl;
					break;
				case Bool:
					cout<<getBool()<<" "<<"Bool"<<endl;
					break;
				case Variable:
					cout<<getVariable()<<" "<<"Variable"<<endl;
					break;
				case Operator:
					cout<<getOperator()<<" "<<"Operator"<<endl;
					break;
				case Bracket:
					cout<<getBracket()<<" "<<"Bracket"<<endl;
					break;
			}
		}

		//Get Functions to fetch values like did in cell class
		DataType getType() const
		{
			return Type;
		}
		int getInt() const
		{
			if(getType()!=Integer)
			{
				string error="Select Operator Exception: Element is not Integer";
				throw error;
			}
			return Data.valueInt;
		}
		double getDouble() const
		{
			if(getType()!=Double)
			{
				string error="Select Operator Exception: Element is not Double";
				throw error;
			}
			return Data.valueDouble;
		}
		char getChar() const
		{
			if(getType()!=Char)
			{
				string error="Select Operator Exception: Element is not Char";
				throw error;
			}
			return Data.valueChar;
		}
		bool getBool() const
		{
			if(getType()!=Bool)
			{
				string error="Select Operator Exception: Element is not Bool";
				throw error;
			}
			return Data.valueBool;
		}
		string getString() const
		{
			if(getType()!=String)
			{
				string error="Select Operator Exception: Element is not String";
				throw error;
			}
			return string(Data.valueString);
		}
		string getVariable() const
		{
			if(getType()!=Variable)
			{
				string error="Select Operator Exception: Element is not Variable";
				throw error;
			}
			return string(Data.valueString);
		}
		string getOperator() const
		{
			if(getType()!=Operator)
			{
				string error="Select Operator Exception: Element is not Operator"+numberToString(getType());
				throw error;
			}
			return string(Data.Op);
		}
		char getBracket() const
		{
			if(getType()!=Bracket)
			{
				string error="Select Operator Exception: Element is not Bracket"+numberToString(getOperator());
				throw error;
			}
			return Data.valueChar;
		}
		string getValue()
		{
			if(getType()==Operator)
				return getOperator();
			else if(getType()==Bracket)
				return numberToString(getBracket());
			else
			{
				string error="Select Operator Exception: Element is neither Bracket nor Operator";
				throw error;
			}

		}

		void setInt(int x)		
		{
			Data.valueInt=x;
			Type=Integer;
		}
		void setDouble(double x)
		{
			Data.valueDouble=x;
			Type=Double;
		}
		void setChar(char x)
		{
			Data.valueChar=x;
			Type=Char;
		}
		void setBool(bool x)
		{
			Data.valueBool=x;
			Type=Bool;
		}
		void setString(string x)
		{
			if(x.length()>=500)
			{
				string error="String Overflow Exception: String Size cannot be greater than 500";
				throw error;
			}

			for(int i=0;i<x.length();i++)
		        Data.valueString[i]=x[i];
		    Data.valueString[x.length()]='\0';
			Type=String;
		}
		bool isInteger(double k) const
		{
		    return k == (double)(int)k;
		}

		//Operator Overloading For +
		QueueObject operator+(const QueueObject& b) const
	    {
	    	QueueObject x;
	    	if(this->getType()==Integer && b.getType()==Integer)
	    		x.setDouble((double)getInt()+(double)b.getInt());
	    	else if(getType()==Double && b.getType()==Double)
	    		x.setDouble(getDouble()+b.getDouble());
	    	else if(getType()==Integer && b.getType()==Double)
	    		x.setDouble(getInt()+b.getDouble());
	    	else if(getType()==Double && b.getType()==Integer)
	    		x.setDouble(getDouble()+b.getInt());
	    	else
	    	{
	    		string error="Select Operation Exception: Addition Of Wrong DataTypes";
				throw error;
	    	}
	    	if(isInteger(x.getDouble()))
	    		x.setInt(x.getDouble());
	        return x;
	    }
	    //Operator Overloading For -
	    QueueObject operator-(const QueueObject& b) const
	    {
	    	QueueObject x;
	    	if(getType()==Integer && b.getType()==Integer)
	    		x.setDouble((double)getInt()-(double)b.getInt());
	    	else if(getType()==Double && b.getType()==Double)
	    		x.setDouble(getDouble()-b.getDouble());
	    	else if(getType()==Integer && b.getType()==Double)
	    		x.setDouble(getInt()-b.getDouble());
	    	else if(getType()==Double && b.getType()==Integer)
	    		x.setDouble(getDouble()-b.getInt());
	    	else
	    	{
	    		string error="Select Operation Exception: Subtraction Of Wrong DataTypes";
				throw error;
	    	}
	    	if(isInteger(x.getDouble()))
	    		x.setInt(x.getDouble());
	        return x;
	    }
	    //Operator Overloading For *
	    QueueObject operator*(const QueueObject& b) const
	    {
	    	QueueObject x;
	    	if(getType()==Integer && b.getType()==Integer)
	    		x.setDouble((double)getInt()*(double)b.getInt());
	    	else if(getType()==Double && b.getType()==Double)
	    		x.setDouble(getDouble()*b.getDouble());
	    	else if(getType()==Integer && b.getType()==Double)
	    		x.setDouble(getInt()*b.getDouble());
	    	else if(getType()==Double && b.getType()==Integer)
	    		x.setDouble(getDouble()*b.getInt());
	    	else
	    	{
	    		string error="Select Operation Exception: Multiplication Of Wrong DataTypes";
				throw error;
	    	}
	    	if(isInteger(x.getDouble()))
	    		x.setInt(x.getDouble());
	        return x;
	    }
	    //Operator Overloading For /
	    QueueObject operator/(const QueueObject& b) const
	    {
	    	if( (b.getType()==Double&&b.getDouble()==0) || (b.getType()==Integer&&b.getInt()==0) )
	    	{
	    		string error="Select Operation Exception: Division by Zero";
				throw error;
	    	}

	    	QueueObject x;
	    	if(getType()==Integer && b.getType()==Integer)
	    		x.setDouble((double)getInt()/(double)b.getInt());
	    	else if(getType()==Double && b.getType()==Double)
	    		x.setDouble(getDouble()/b.getDouble());
	    	else if(getType()==Integer && b.getType()==Double)
	    		x.setDouble((double)getInt()/b.getDouble());
	    	else if(getType()==Double && b.getType()==Integer)
	    		x.setDouble(getDouble()/(double)b.getInt());
	    	else
	    	{
	    		string error="Select Operation Exception: Division Of Wrong DataTypes";
				throw error;
	    	}
	    	if(isInteger(x.getDouble()))
	    		x.setInt(x.getDouble());
	        return x;
	    }
	    //Operator Overloading For <
	    QueueObject operator<(const QueueObject& b) const
	    {
	    	QueueObject x;
	    	if(getType()==Integer && b.getType()==Integer)
	    		x.setBool(getInt() < b.getInt());
	    	else if(getType()==Double && b.getType()==Double)
	    		x.setBool(getDouble() < b.getDouble());
	    	else if(getType()==Integer && b.getType()==Double)
	    		x.setBool(getInt() <  b.getDouble());
	    	else if(getType()==Double && b.getType()==Integer)
	    		x.setBool(getDouble() < b.getInt());
	    	else if(getType()==Char && b.getType()==Char)
	    		x.setBool(getChar() < b.getChar());
	    	else if(getType()==String && b.getType()==String)
	    		x.setBool(getString() < b.getString());
	    	else
	    	{
	    		string error="Select Operation Exception: Comparison Of Wrong DataTypes";
				throw error;
	    	}
	        return x;
	    }
	    //Operator Overloading For >
	    QueueObject operator>(const QueueObject& b) const
	    {
	    	QueueObject x;
	    	if(getType()==Integer && b.getType()==Integer)
	    		x.setBool(getInt() > b.getInt());
	    	else if(getType()==Double && b.getType()==Double)
	    		x.setBool(getDouble() > b.getDouble());
	    	else if(getType()==Integer && b.getType()==Double)
	    		x.setBool(getInt() >  b.getDouble());
	    	else if(getType()==Double && b.getType()==Integer)
	    		x.setBool(getDouble() > b.getInt());
	    	else if(getType()==Char && b.getType()==Char)
	    		x.setBool(getChar() > b.getChar());
	    	else if(getType()==String && b.getType()==String)
	    		x.setBool(getString() > b.getString());
	    	else
	    	{
	    		string error="Select Operation Exception: Comparison Of Wrong DataTypes";
				throw error;
	    	}
	        return x;
	    }
	    //Operator Overloading For >=
	    QueueObject operator>=(const QueueObject& b) const
	    {
	    	QueueObject x;
	    	if(getType()==Integer && b.getType()==Integer)
	    		x.setBool(getInt() >= b.getInt());
	    	else if(getType()==Double && b.getType()==Double)
	    		x.setBool(getDouble() >= b.getDouble());
	    	else if(getType()==Integer && b.getType()==Double)
	    		x.setBool(getInt() >=  b.getDouble());
	    	else if(getType()==Double && b.getType()==Integer)
	    		x.setBool(getDouble() >= b.getInt());
	    	else if(getType()==Char && b.getType()==Char)
	    		x.setBool(getChar() >= b.getChar());
	    	else if(getType()==String && b.getType()==String)
	    		x.setBool(getString() >= b.getString());
	    	else
	    	{
	    		string error="Select Operation Exception: Comparison Of Wrong DataTypes";
				throw error;
	    	}
	        return x;
	    }
	    //Operator Overloading For <=
	    QueueObject operator<=(const QueueObject& b) const
	    {
	    	QueueObject x;
	    	if(getType()==Integer && b.getType()==Integer)
	    		x.setBool(getInt() <= b.getInt());
	    	else if(getType()==Double && b.getType()==Double)
	    		x.setBool(getDouble() <= b.getDouble());
	    	else if(getType()==Integer && b.getType()==Double)
	    		x.setBool(getInt() <=  b.getDouble());
	    	else if(getType()==Double && b.getType()==Integer)
	    		x.setBool(getDouble() <= b.getInt());
	    	else if(getType()==Char && b.getType()==Char)
	    		x.setBool(getChar() <= b.getChar());
	    	else if(getType()==String && b.getType()==String)
	    		x.setBool(getString() <= b.getString());
	    	else
	    	{
	    		string error="Select Operation Exception: Comparison Of Wrong DataTypes";
				throw error;
	    	}
	        return x;
	    }
	    //Operator Overloading For ==
	    QueueObject operator==(const QueueObject& b) const
	    {
	    	QueueObject x;
	    	if(getType()==Integer && b.getType()==Integer)
	    		x.setBool(getInt() == b.getInt());
	    	else if(getType()==Double && b.getType()==Double)
	    		x.setBool(getDouble() == b.getDouble());
	    	else if(getType()==Integer && b.getType()==Double)
	    		x.setBool(getInt() ==  b.getDouble());
	    	else if(getType()==Double && b.getType()==Integer)
	    		x.setBool(getDouble() == b.getInt());
	    	else if(getType()==Char && b.getType()==Char)
	    		x.setBool(getChar() == b.getChar());
	    	else if(getType()==String && b.getType()==String)
	    		x.setBool(getString() == b.getString());
	    	else if(getType()==Bool && b.getType()==Bool)
	    		x.setBool(getBool() == b.getBool());
	    	else
	    	{
	    		string error="Select Operation Exception: Comparison Of Wrong DataTypes";
				throw error;
	    	}
	        return x;
	    }
	    //Operator Overloading For !=
	    QueueObject operator!=(const QueueObject& b) const
	    {
	    	QueueObject x;
	    	if(getType()==Integer && b.getType()==Integer)
	    		x.setBool(getInt() != b.getInt());
	    	else if(getType()==Double && b.getType()==Double)
	    		x.setBool(getDouble() != b.getDouble());
	    	else if(getType()==Integer && b.getType()==Double)
	    		x.setBool(getInt() !=  b.getDouble());
	    	else if(getType()==Double && b.getType()==Integer)
	    		x.setBool(getDouble() != b.getInt());
	    	else if(getType()==Char && b.getType()==Char)
	    		x.setBool(getChar()!= b.getChar());
	    	else if(getType()==String && b.getType()==String)
	    		x.setBool(getString() != b.getString());
	    	else if(getType()==Bool && b.getType()==Bool)
	    		x.setBool(getBool() != b.getBool());
	    	else
	    	{
	    		string error="Select Operation Exception: Comparison Of Wrong DataTypes";
				throw error;
	    	}
	        return x;
	    }
	    //Operator Overloading For &
	    QueueObject operator&(const QueueObject& b) const
	    {
	    	QueueObject x;
	    	if(getType()==Bool && b.getType()==Bool)
	    		x.setBool(getBool() && b.getBool());
	    	else
	    	{
	    		string error="Select Operation Exception: Comparison Of Wrong DataTypes";
				throw error;
	    	}
	        return x;
	    }
	    //Operator Overloading For |
	    QueueObject operator|(const QueueObject& b) const
	    {
	    	QueueObject x;
	    	if(getType()==Bool && b.getType()==Bool)
	    		x.setBool(getBool() || b.getBool());
	    	else
	    	{
	    		string error="Select Operation Exception: Comparison Of Wrong DataTypes";
				throw error;
	    	}
	        return x;
	    }
	    //Operator Overloading For !
	    QueueObject operator!() 
	    {
	    	QueueObject x;
	    	if(getType()==Bool)
	    		x.setBool(!getBool());
	    	else
	    	{
	    		string error="Select Operation Exception: Negation Of Wrong DataType";
				throw error;
	    	}
	        return x;
	    }
	};
	//operator weights for evaulating predicate
	int operaterWeight(string op)
	{
		if(op=="!")
			return 5;
		if(op=="*"||op=="/")
			return 4;
		if(op=="+"||op=="-")
			return 3;
		if(op=="=="||op=="!="||op==">"||op=="<"||op==">="||op=="<=")
			return 2;
		if(op=="&")
			return 1;
		if(op=="|")
			return 0;
	}
	//precedence check for operators
	bool hasPrecendence(string op1, string op2)
	{
		int op1Weight= operaterWeight(op1);
		int op2Weight= operaterWeight(op2);
		if(op2Weight>=op1Weight)
			return true;
		return false;
	}
	QueueObject applyOperation(QueueObject Operator, QueueObject Operand)
	{
		if(Operator.getOperator()=="!")
			return !Operand;
	}

	QueueObject applyOperation(QueueObject Operator, QueueObject OperandB, QueueObject OperandA)
	{
		if(Operator.getOperator()=="+")
			return OperandA+OperandB;
		if(Operator.getOperator()=="-")
			return OperandA-OperandB;
		if(Operator.getOperator()=="*")
			return OperandA*OperandB;
		if(Operator.getOperator()=="/")
			return OperandA/OperandB;
		if(Operator.getOperator()=="<")
			return OperandA<OperandB;
		if(Operator.getOperator()==">")
			return OperandA>OperandB;
		if(Operator.getOperator()=="<=")
			return OperandA<=OperandB;
		if(Operator.getOperator()==">=")
			return OperandA>=OperandB;
		if(Operator.getOperator()=="==")
			return OperandA==OperandB;
		if(Operator.getOperator()=="!=")
			return OperandA!=OperandB;
		if(Operator.getOperator()=="&")
			return OperandA&OperandB;
		if(Operator.getOperator()=="|")
			return OperandA|OperandB;
	}
	//Function to validate parsed select query
	bool validateParsedObjects(vector<QueueObject> objects)
	{
		for(int i=0;i<objects.size()-1;i++)
		{
			if((i==0 && objects[i].getType()==Operator && objects[i].getOperator()!="!") || (i==objects.size()-2 && objects[i+1].getType()==Operator))
			{
				string error="Select Operation Exception: Binary Operator should have two operands";
				throw error;
			}
			else if(objects[i].getType()==Operator && objects[i+1].getType()==Operator && objects[i+1].getOperator()!="!")
			{
				string error="Select Operation Exception: Operators should have operands in between them";
				throw error;
			}
			else if( (objects[i].getType()==Bracket)&&(objects[i+1].getType()==Bracket) && ((objects[i].getBracket()=='(' && objects[i+1].getBracket()==')')||(objects[i].getBracket()==')' && objects[i+1].getBracket()=='('))  )
			{
				string error="Select Operation Exception: Brackets () should enclose some expression";
				throw error;
			}
		}

		for(int i=1;i<objects.size()-1;i++)
		{
			if(objects[i].getType()==Operator)
			{
				if((objects[i-1].getType()==Bracket && objects[i-1].getBracket()=='(') || (objects[i+1].getType()==Bracket && objects[i+1].getBracket()==')'))
				{
					string error="Select Operation Exception: Brackets and Operators Wrongly Placed";
					throw error;
				}
			}
		}

		return true;
	}
	//Returns QueueObjects after parsing the predicate
	vector<QueueObject> parse(string predicate)
	{
		string expression = trim(predicate);
		vector<QueueObject> objects;
		objects.clear();
		if(expression=="" || expression=="*")
		{
			return objects;
		}
		int bracketCount=0;
		for(int i=0;i<expression.size();i++)
		{
			if(expression[i]==' ')
				continue;
			else if(expression[i]=='(' || expression[i]==')')
			{
				objects.push_back(QueueObject(numberToString(expression[i]),"Bracket"));
				if(expression[i]=='(')
					bracketCount++;
				else
					bracketCount--;
			}
			else if(expression[i]=='+' || expression[i]=='-' || expression[i]=='*' || expression[i]=='/' || expression[i]=='<' || expression[i]=='>' || expression[i]=='=' || expression[i]=='!')
			{
				if((expression[i]=='<'||expression[i]=='>'||expression[i]=='='||expression[i]=='!') && (i<expression.size()-1) && expression[i+1]=='=')
				{
					string x="";
					x+=expression[i];
					x+=expression[i+1];
					objects.push_back(QueueObject(x,"Operator"));
					i++;
				}
				else if(expression[i]=='=')
				{
					objects.push_back(QueueObject("==","Operator"));
				}
				else
				{
					objects.push_back(QueueObject(numberToString(expression[i]),"Operator"));
				}
			}
			else if(expression[i]=='|')
			{
				objects.push_back(QueueObject(numberToString(expression[i]),"Operator"));
				if((i<expression.size()-1) && expression[i+1]=='|')
					i++;
			}
			else if(expression[i]=='&')
			{
				// cout<<"hello"<<endl;
				// cout<<expression[i+1]<<endl;
				objects.push_back(QueueObject(numberToString(expression[i]),"Operator"));
				if((i<expression.size()-1) && expression[i+1]=='&')
				{
					i++;
				}
			}
			else if(expression[i]=='\"')
	        {
	        	int count=0;
	        	int j=i;
	        	i++;
	        	while(i<expression.size() && expression[i]!='\"')
	        	{
	        		count++;
	        		i++;
	        	}
	        	if(i==expression.size())
	        	{
	        		string error="Select Parse Predicate Error: String Not Terminated With \"";
	        		throw error;
	        	}
	        	objects.push_back(QueueObject(expression.substr(j+1,count),"String"));
	        }
	        else if(expression[i]=='\'')
	        {
	        	if(i+2>=expression.size() || expression[i+2]!='\'')
	        	{
	        		string error="Select Parse Predicate Error: Char Not Terminated With \'";
	        		throw error;
	        	}
	        	else
	        	{
	        		objects.push_back(QueueObject(numberToString(expression[i+1]),"Char"));
	        		i+=2;
	        	}
	        }
	        else if((expression[i]>='0')&&(expression[i]<='9')||(expression[i]=='.'))
	        {
				int count=1;
				int j=i;
				i++;
				while(i<expression.size()&&((expression[i]>='0')&&(expression[i]<='9')||(expression[i]=='.')))        	
				{
					count++;
					i++;
				}
				i--;
				string number = expression.substr(j,count);
				if(std::count(number.begin(),number.end(),'.')==0)
					objects.push_back(QueueObject(number,"Integer"));
				else
					objects.push_back(QueueObject(number,"Double"));
	        }
	        else if(((expression[i]>='a')&&(expression[i]<='z'))||((expression[i]>='A')&&(expression[i]<='Z'))) 
	        {
	        	if((expression[i]=='t' || expression[i]=='T') && i+3<expression.size() && expression.substr(i+1,3)=="rue")
		        {
		        		objects.push_back(QueueObject("True","Bool"));
		        		i+=3;
	        	}
	        	else if((expression[i]=='f' || expression[i]=='F') && i+4<expression.size() && (expression.substr(i+1,4)=="alse"))
	        	{
	        		objects.push_back(QueueObject("False","Bool"));
	        		i+=4;
	        	}
	        	else
	        	{
		        	int count=1;
		        	int j=i;
		            i++;
		            while(i<expression.size()&&((expression[i]>='a')&&(expression[i]<='z'))||((expression[i]>='A')&&(expression[i]<='Z')))
		            {
		                count++;
		                i++;
		            }
		            i--;
		            objects.push_back(QueueObject(expression.substr(j,count),"Variable"));
	        	}
	        }
	        else
	        {
	        	string error="Select Parse Predicate Error: Invalid Query Parsed "+numberToString(expression[i]);
	    		throw error;
	        }

	        if(bracketCount<0)
	        {
	        	string error="Select Parse Predicate Error: ) Bracket Passed Before (";
	    		throw error;
	        }

		}

		if(bracketCount!=0)
		{
			string error="Select Parse Predicate Error: Query with misbalanced paranthesis provided.";
			throw error;
		}

		if(validateParsedObjects(objects))
			return objects;
	}

	bool satisfiesPredicate(vector<QueueObject> objects, Table A, std::vector<cell> dataTuple)
	{
		if(objects.size()==0)
			return true;

		stack<QueueObject> Operands;
		stack<QueueObject> Operators;

		for(int i=0;i<objects.size();i++)
		{
			if(objects[i].getType()==Variable)
			{
				int index=A.getAttributeIndexByName(objects[i].getVariable());
				Operands.push(QueueObject(dataTuple[index]));
			}
			else if(objects[i].getType()==Bracket)
			{
				if(objects[i].getValue()=="(")
					Operators.push(objects[i]);
				else
				{
					while(!Operators.empty() && Operators.top().getValue()!="(")
					{
						QueueObject op = Operators.top();
						Operators.pop();
						if(op.getValue()=="!")
						{
							QueueObject operandA = Operands.top();
							Operands.pop();
							Operands.push(applyOperation(op,operandA));
						}
						else
						{
							QueueObject operandA = Operands.top();
							Operands.pop();
							QueueObject operandB = Operands.top();
							Operands.pop();
							Operands.push(applyOperation(op,operandA,operandB));
						}
					}
					Operators.pop();
				}
			}
			else if(objects[i].getType()==Operator)
			{
				while(!Operators.empty() && Operators.top().getValue()!="(" && hasPrecendence(objects[i].getOperator(),Operators.top().getValue()))
				{
					

					QueueObject op = Operators.top();
					Operators.pop();
					if(op.getValue()=="!")
					{
						QueueObject operandA = Operands.top();
						Operands.pop();
						Operands.push(applyOperation(op,operandA));
					}
					else
					{
						QueueObject operandA = Operands.top();
						Operands.pop();
						QueueObject operandB = Operands.top();
						Operands.pop();
						Operands.push(applyOperation(op,operandA,operandB));
					}
				}
				Operators.push(objects[i]);
			}
			else
			{
				Operands.push(objects[i]);
			}
		}

		while(!Operators.empty())
		{
			QueueObject op = Operators.top();
			Operators.pop();
			if(op.getValue()=="!")
			{
				QueueObject operandA = Operands.top();
				Operands.pop();
				Operands.push(applyOperation(op,operandA));
			}
			else
			{
				QueueObject operandA = Operands.top();
				Operands.pop();
				QueueObject operandB = Operands.top();
				Operands.pop();
				Operands.push(applyOperation(op,operandA,operandB));
			}
		}

		return Operands.top().getBool();
	}

public:
	//Driver Function For Select Query
	//Input: Table A, string predicate
	//Output: Table with entries that satisfies the predicate
	Table SelectFromTable(Table A, string predicate)
	{
		vector<QueueObject> objects = parse(predicate);    		//Seperate the predicate into well defined QueueObjects with each having their datatypes 
		Table output = A;
		output.setTableName(A.getTableName());
		output.dataInTable.clear();
		std::set<std::vector<cell> >::iterator it;
		for(it = A.dataInTable.begin(); it != A.dataInTable.end(); it++)
		{
			vector<cell> dataTuple = *it;
			if(satisfiesPredicate(objects,A,dataTuple))				// If the datat tuple satisfies the condition, then add it to new table
				output.addDataTuple(dataTuple);
		}
		return output;
	}
};

//Function to check for balanced parenthesis in the query parsed
// Valid Entry: {([{}])}({{{[]}}})
//Invalid Entry: ][[][]()[]]
bool checkBalancedParenthesis(string query)
{
	stack<string> brackets;														//Check the pair of brackets by pushing and poping from the stack
	for(int i=0;i<query.size();i++)
	{
		if(query[i]=='(' || query[i]=='{' || query[i]=='[')
			brackets.push(numberToString(query[i]));
		else if(query[i]==')')
		{
			if(!brackets.empty() && brackets.top()=="(")						//If it matches the same type, else error
				brackets.pop();
			else
				return false;
		}
		else if(query[i]=='}')
		{
			if(!brackets.empty() && brackets.top()=="{")						//If it matches the same type, else error
				brackets.pop();
			else
				return false;
		}
		else if(query[i]==']')
		{
			if(!brackets.empty() && brackets.top()=="[")						//If it matches the same type, else error
				brackets.pop();
			else
				return false;
		}
	}

	return true;
}

//Function to remove extra parenthesis enclosing a string
//Input: {[[abc, () , def]]}
//Output: abc, () , def
string removeExtraParenthese(string query)
{
	while(query[0]=='(' || query[0]=='[' || query[0]=='{')
		query = query.substr(1,query.size()-2);
	return query;
}

//Function to check if string starts with a sbustring or not
//Input: s1="abcdef" s2="abcd"
//Output: True
bool starts_with(string s1, string s2)
{
    return s2.size() <= s1.size() && s1.compare(0, s2.size(), s2) == 0;
}

//CLass Database which stores all the tables in the relational algebra parser
class Database
{
public:
	vector<Table> Tables;												// Vector of Tables objects. All tables are stored here.
	map<string, int> tableIndexMap;										// Names of Tables mapped with the index in Vector Tables
	int noOfTables;														// No Of Tables in The Database

	//Initialise dataabse with zero tables
	Database()
	{
		noOfTables=0;
	}

	// Adds a table to the database
	void CreateTable(string tableName, vector<string> attributes, vector<string> attributes_types)
	{
		if(tableExists(tableName))				//Duplicate Table name check
		{
			string error="Table Create Error: Table Already Exists With Name: "+tableName;
			throw error;
		}
		if(attributes.size()!=attributes_types.size())						
		{
			string error="Attribute Match Error: All Attributes should have a type";
			throw error;
		}
		Table newTable(tableName,attributes.size(),attributes,attributes_types);
		Tables.push_back(newTable);
		tableIndexMap[tableName]=noOfTables;
		noOfTables++;
	}

	//Show all tables in the database
	void showTables()
	{
		for(int i=0;i<Tables.size();i++)
			cout<<i+1<<". "<<Tables[i].getTableName()<<endl;
	}
	//Check if table exists in Database
	bool tableExists(string name)
	{
		if(tableIndexMap.count(name)>0)
			return true;
		return false;
	}
	//returns the index of string name in the database
	int getTableIndexByName(string name)
	{
		if(!tableExists(name))
		{
			string error="Table Name Error: No Table With Name: "+name;
			throw error;
		}
		return tableIndexMap[name];
	}

	//Returns the pointer to the table with table name as name
	Table& getTableByName(string name)
	{
		if(!tableExists(name))
		{
			string error="Table Name Error: No Table With Name: "+name;
			throw error;
		}
		int index = tableIndexMap[name];
		return Tables[index];
	}

	//Clear the database
	void clearDataBase()
	{
		Tables.clear();
		tableIndexMap.clear();
	}
	
}dataBase;											//Creates an object dataBase of Class Database


//Describes the schema of table "name"
void DescribeTable(string name)
{
	name=trim(name.substr(4));
	if(dataBase.tableExists(name))
		dataBase.getTableByName(name).showSchema();
	else
		throwError("Invalid Query: No Table Found Named \""+name+"\"");
}

//Query Parser for six basic operations
//Input: string query
//Output: Returns a Table according to the query
Table QueryParser(string query)
{
	query = trim(query);   													//trims the trailing spaces from front and back of string
	if(query=="")															//NULL Query Check
		throwError("Query Parser Error: NULL Query Parsed");
	if(!checkBalancedParenthesis(query))									//MisBalanced Parenthesis Check
		throwError("Query Parser Error: Misbalanced Bracket Query Parsed");
	
	query = removeExtraParenthese(query);									//Remove Extra Brackets
	if(dataBase.tableExists(query))											//If table exists in dataabse then return table, otherwise nested query
		return dataBase.getTableByName(query);
	int firstCurlyBracket = query.find("{");								//Find position of first '{'
	int firstSquareBracket = query.find("[");								//Find position of first '['

	if(firstSquareBracket==string::npos && firstCurlyBracket==string::npos)			//if both not found then error
		throwError("Query Parser Error: Invalid Query Or Subquery");

	int flag;
	if(firstSquareBracket!=string::npos && firstCurlyBracket!=string::npos)
	{
		if(firstCurlyBracket<firstSquareBracket)
			flag=0;
		else flag=1;
	}
	else if(firstCurlyBracket==string::npos)
		flag=1;
	else
		throwError("Query Parser Error: Proper Brackets Are Not Provided In Your Query Or Subquery");

	string checkQuery = query;
	transform(checkQuery.begin(), checkQuery.end(), checkQuery.begin(), ::toupper);
	if(flag==1 && (starts_with(checkQuery,"SELECT")||starts_with(checkQuery,"RENAME")||starts_with(checkQuery,"PROJECT")))
		throwError("Query Parser Error: Proper Brackets Are Not Provided In Your Query Or Subquery");
	if(flag==0 && (starts_with(checkQuery,"UNION")||starts_with(checkQuery,"CART")||starts_with(checkQuery,"SDIFF")))
		throwError("Query Parser Error: Proper Brackets Are Not Provided In Your Query Or Subquery");
	
	if(flag==0)
	{
		int secondCurlyBracket = query.find("}",firstCurlyBracket);
		if(secondCurlyBracket==string::npos || trim(query.substr(secondCurlyBracket+1,firstSquareBracket-secondCurlyBracket-1))!="" || query[query.size()-1]!=']')
			throwError("Query Parser Error: Proper Brackets Are Not Provided In Your Query Or Subquery");

		string queryFunction = trim(query.substr(0,firstCurlyBracket));				//extract queryfunction from query
		transform(queryFunction.begin(), queryFunction.end(), queryFunction.begin(), ::toupper);		//Transform it to UPPERCASE
		string betweenBraces = trim(query.substr(firstCurlyBracket+1,secondCurlyBracket-firstCurlyBracket-1));  //extract { } part
		string betweenBrackets = trim(query.substr(firstSquareBracket+1,query.size()-firstSquareBracket-2));	//extract [ ] part

		Table A;				

		if(dataBase.tableExists(betweenBrackets))							//If table exists in dataabse then return table, otherwise nested query
			A = dataBase.getTableByName(betweenBrackets);
		else
			A = QueryParser(betweenBrackets);

		if(queryFunction=="RENAME")
		{
			int pipe = betweenBraces.find('|');
			if(pipe==string::npos)
			{
				return RenameTable(A,betweenBraces);
			}
			int secondPipe = betweenBraces.substr(pipe+1).find('|');
			if(secondPipe!=string::npos)
				throwError("Query Parser Error: Two '|' Are Provided In Your Query Or Subquery");

			else
			{
				string tableName=trim(betweenBraces.substr(0,pipe));
				vector<string> attributes = commaSeparatedStrings(betweenBraces.substr(pipe+1),',');
				return RenameTable(A,tableName,attributes);
			}
		}
		else if(queryFunction=="PROJECT")
		{
			vector<string> attributes = commaSeparatedStrings(betweenBraces,',');
			return ProjectTable(A,attributes);
		}
		else if(queryFunction=="SELECT")
		{
			SelectParser a;
			return a.SelectFromTable(A,betweenBraces);
		}
		else
			throwError("Query Parser Error: Wrong Query Given ");

	}
	else if(flag==1)
	{
		if(query[query.size()-1]!=']')
			throwError("Query Parser Error: Proper Brackets Are Not Provided In Your Query Or Subquery");
		
		string betweenBrackets = trim(query.substr(firstSquareBracket+1,query.size()-firstSquareBracket-2));	// extract [ ] part
		string queryFunction = trim(query.substr(0,firstSquareBracket));			//extract query function
		transform(queryFunction.begin(), queryFunction.end(), queryFunction.begin(), ::toupper);				//Convert to UPPERCASE
		//Table A B
		int count=0;
		int indexOfComma=-1;
		for(int i=0;i<betweenBrackets.size();i++)
		{
			if(betweenBrackets[i]=='(' || betweenBrackets[i]=='{' || betweenBrackets[i]=='[')
			{
				count++;
				continue;
			}
			if(betweenBrackets[i]==')' || betweenBrackets[i]=='}' || betweenBrackets[i]==']')
			{
				count--;
				continue;
			}
			if(betweenBrackets[i]==',' && count == 0 && indexOfComma==-1)
			{
				indexOfComma=i;
			}
			else if(betweenBrackets[i]==',' && count == 0 && indexOfComma!=-1)
				throwError("Query Parser Error: Query Not Given In Proper Format");

		}
		if(indexOfComma==-1 || indexOfComma==0 || indexOfComma==betweenBrackets.size()-1)
			throwError("Query Parser Error: Query Not Given In Proper Format");

		string queryA = trim(betweenBrackets.substr(0,indexOfComma));			//split strings about comma
		string queryB = trim(betweenBrackets.substr(indexOfComma+1));
		if(queryA==""||queryB=="")
			throwError("Query Parser Error: Wrong Query Given");

		Table A,B;
		if(dataBase.tableExists(queryA))
			A = dataBase.getTableByName(queryA);
		else
			A = QueryParser(queryA);
		if(dataBase.tableExists(queryB))
			B = dataBase.getTableByName(queryB);
		else
			B = QueryParser(queryB);

		if(queryFunction=="CART")
		{
			return CartesianProduct(A,B);
		}
		else if(queryFunction=="UNION")
		{
			return UnionTables(A,B);
		}
		else if(queryFunction=="SDIFF")
		{
			return SetDifferenceTable(A,B);
		}
		else
			throwError("Query Parser Error: Wrong Query Given");

	}
	else
		throwError("Query Parser Error: Wrong Query Given");

}

void loadData()
{
	fstream fileObject;
	fileObject.open("TablesInfo.csv", ios::in);					//load csv file
	if(!fileObject.is_open())
	{
		throw "Error Opening File TablesInfo.csv, Please Retry...";
	}

	int tableOpen=0, istableNameSet=0;
	int schemaOpen=0,isSchemaSet=0;
	string tableName="";
	vector<string> attributes;
	vector<string> attributes_types;
	vector<vector<string> > dataForTable;
	string line;
	while(getline(fileObject,line))
	{
		// cout<<line<<endl;
		line=trim(line);
		if(line=="")
			continue;
		else if(line=="<Table>")					//Error Check
		{
			if(tableOpen==0 && schemaOpen==0 && isSchemaSet==0 && istableNameSet==0)
				tableOpen=1;
			else
				throwError("Error In TablesInfo.csv");
		}
		else if(line=="</Table>")											//Error Check
		{
			if(tableOpen==1 && schemaOpen==0 && isSchemaSet==1 && istableNameSet==1)
			{
				dataBase.CreateTable(tableName,attributes,attributes_types);							// If </Table> then save table to Database
				for(int i=0;i<dataForTable.size();i++)
					dataBase.getTableByName(tableName).insertIntoTable(dataForTable[i]);
				tableName="";
				attributes.clear();
				attributes_types.clear();
				dataForTable.clear();
				tableOpen=0;
				istableNameSet=0;
				isSchemaSet=0;
			}
			else
				throwError("Error In TablesInfo.csv");
		}
		else if(line=="<Schema>")
		{
			if(tableOpen==1 && isSchemaSet==0 && schemaOpen==0 && istableNameSet==1)
				schemaOpen=1;
			else
				throwError("Error In TablesInfo.csv");									//Error Check
		}
		else if(line=="</Schema>")
		{
			if(tableOpen==1 && schemaOpen==1 && istableNameSet ==1 && isSchemaSet==0)
			{
				isSchemaSet=1;
				schemaOpen=0;
			}
			else
				throwError("Error In TablesInfo.csv");										//Error Check
		}
		else if(tableOpen == 1 && isSchemaSet ==0 && schemaOpen==0 && istableNameSet==0)
		{
			tableName = line;
			istableNameSet=1;
		}
		else if(tableOpen==1 && istableNameSet==1 && schemaOpen==1 && isSchemaSet==0)
		{
			vector <string> att = commaSeparatedStrings(line,',');
			if(att.size()!=2)
				throwError("Error In TablesInfo.csv");
			att[0]=trim(att[0]);
			att[1]=trim(att[1]);
			if(att[0]=="" || att[1]=="")
				throwError("Error In TablesInfo.csv");
			attributes.push_back(att[0]);
			attributes_types.push_back(att[1]);
		}
		else if(tableOpen==1 && istableNameSet==1 && schemaOpen==0 && isSchemaSet==1)
		{
			vector<string> dataTuple = commaSeparatedStrings(line,'|');
			if(dataTuple.size()!=attributes.size())
				throwError("Error In TablesInfo.csv");
			dataForTable.push_back(dataTuple);
		}
		else
		{
				throwError("Error In TablesInfo.csv");

		}

	}
}

int main()
{

	try
	{

		loadData();				//load Data
		cout<<"Data Loaded..."<<endl;
		// for(int i=0;i<dataBase.Tables.size();i++)				//Show data of all tables
		// 	dataBase.Tables[i].showData();

		while(1)
		{
			try
			{
				cout<<">>> ";
				string query;
				getline(cin,query);
				string queryCheck = trim(query);
				transform(queryCheck.begin(), queryCheck.end(), queryCheck.begin(), ::toupper);
				if(queryCheck=="SHOW TABLES")
					dataBase.showTables();

				else if(queryCheck=="EXIT")
				{
					cout<<"Bye Exiting...."<<endl;
					return 0;
				}
				else if(starts_with(queryCheck,"DESC"))
				{
					DescribeTable(query);
				}
				else if(starts_with(queryCheck,"PROJECT")||starts_with(queryCheck,"SELECT")||starts_with(queryCheck,"RENAME")||starts_with(queryCheck,"UNION")||starts_with(queryCheck,"CART")||starts_with(queryCheck,"SDIFF"))
				{
					QueryParser(query).showData();
				}
				else
					throwError("Invalid Query");
			}
			catch(const string msg)
			{
				cout<<msg <<endl;
			}
		}

	}
	catch(const string msg)
	{
		cerr<<msg <<endl;
	}
}