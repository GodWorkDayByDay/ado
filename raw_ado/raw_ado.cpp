// raw_ado.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <iostream>

#import "C:\Program Files\Common Files\System\ADO\msado15.dll" rename("EOF", "EndOfFile") no_namespace

int main(int argc, char* argv[])
{
	if ( FAILED( ::CoInitialize( NULL ) ) ) {
		_tprintf( _T( "::CoInitialize Fail!!" ) );
	}

	try
	{
		_ConnectionPtr conn;
		_CommandPtr command;
		conn.CreateInstance( __uuidof( Connection ) );
		command.CreateInstance( __uuidof( Command ) );

		conn->Open( "Driver={MySQL ODBC 5.3 ANSI Driver};Server=192.168.3.171;Port=3306;Database=test;User=root;Password=123456;charset=utf8;Option=8;no_ssps=1", "", "", NULL );
		command->ActiveConnection = conn;

		int val = 2000;
		VARIANT vt;
		vt.vt = VT_I2;
		_ParameterPtr parameter = command->CreateParameter( "@Param1", adInteger, adParamOutput, sizeof(val) );
		command->Parameters->Append( parameter );
		parameter->Value = static_cast< _variant_t >( &val );

		command->CommandType = adCmdText;
		command->CommandText = "call sample_proc2(@Param1)";

		_RecordsetPtr recordset = command->Execute( NULL, NULL, adOptionUnspecified );

		while ( !recordset->GetEndOfFile() )
		{
			_variant_t& vFieldValue = recordset->GetCollect( "Param1" );
			val = vFieldValue;
			std::cout << "out : " << val << std::endl;
			recordset->MoveNext();
		}


		//_CommandPtr command2;
		////command2->ActiveConnection = conn;
		//command->CommandType = adCmdText;
		//command->CommandText = "select @Param1 as param1";

		//_RecordsetPtr recordset3 = command->Execute( NULL, NULL, adOptionUnspecified );
		//while ( !recordset->GetEndOfFile() )
		//{
		//	_variant_t& vFieldValue = recordset3->GetCollect( "@Param1" );
		//	val = vFieldValue;
		//	std::cout << "out : " << val << std::endl;
		//	recordset->MoveNext();
		//}






		_variant_t& vFieldValue1 = command->Parameters->GetItem( "@Param1" )->Value;

		val = vFieldValue1;
		std::cout << "out : " << val << std::endl;

		if ( !recordset )
			return 0;


		_variant_t& vFieldValue = command->Parameters->GetItem( "@Param1" )->Value;

		_variant_t variantRec;
		variantRec.intVal = 0;
		_RecordsetPtr recordset2 = recordset->NextRecordset( ( _variant_t* ) &variantRec );
		
		

		val = vFieldValue;
		std::cout << "out : " << val << std::endl;

	}
	catch ( _com_error& e )
	{
		printf("%s\n", (char*)e.Description());
	}
		

	
	return 0;
}

