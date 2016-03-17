#include <thread>
#include "ado.h"

#undef max
#undef min
#include <limits>

#define MAX_STR_LEN 30
#define MAX_BIN_LEN 50

using namespace ADO_NS;

int sample_procedure_call();
int sample_raw_query();



int _tmain(int argc, _TCHAR* argv[])
{	
	CONFIG config(_T("127.0.0.1,1433"), NULL, _T("sa"), _T("1234"));
	//CONFIG config(NULL, _T("test_dsn"), _T("sa"), _T("1234"));
	config.SetInitCatalog(_T("TEST"));
	config.SetConnectionTimeout(3);
	config.SetRetryConnection(true);

	ADO::Initialize(config);
	
	std::thread t1(&sample_raw_query);
	sample_procedure_call();
	t1.join();

	ADO::Free();
	return 0;
}



int sample_procedure_call()
{
	int iReturnVal = 0;
	bool bitVal = true;
	char tinyVal = std::numeric_limits<char>::max();
	short smallVal = std::numeric_limits<short>::max();
	int intVal = std::numeric_limits<int>::max();
	__int64 bigVal = std::numeric_limits<__int64>::max();
	float floatVal = 3.1415926f;
	TCHAR charVal[MAX_STR_LEN] = _T("This is string!!");
	COleDateTime smallDateTimeVal(2016, 03, 07, 11, 12, 11), dateTimeVal(2016, 03, 07, 11, 12, 11);
	BYTE binaryVal[10] = { 0XFF, 0XFE, 0X00, 0X01, 0X06, 0X07, };

	bool bitField = false; int intField = 0; COleDateTime dateTimeField;
	TCHAR charField[MAX_STR_LEN] = { 0, };
	BYTE binaryValField[MAX_BIN_LEN]; int nSize = 0;
	memset(binaryValField, 0x00, MAX_BIN_LEN);
	

	try
	{
		auto pDB = ADO::GetDB();
		if (!pDB)
			return 0;

		pDB->CreateParameter(_T("return"), adParamReturnValue, iReturnVal);
		pDB->CreateParameter(_T("@io_bitVal"), adParamInputOutput, bitVal);
		pDB->CreateParameter(_T("@io_tinyVal"), adParamInputOutput, tinyVal);
		pDB->CreateParameter(_T("@io_smallVal"), adParamInputOutput, smallVal);
		pDB->CreateParameter(_T("@io_intVal"), adParamInputOutput, intVal);
		pDB->CreateParameter(_T("@io_bigVal"), adParamInputOutput, bigVal);
		pDB->CreateParameter(_T("@io_floatVal"), adParamInputOutput, floatVal);
		pDB->CreateParameter(_T("@io_charVal"), adParamInputOutput, charVal, MAX_STR_LEN + 1);
		pDB->CreateParameter(_T("@io_smalldatetimeVal"), adParamInputOutput, smallDateTimeVal);
		pDB->CreateParameter(_T("@io_datetimeVal"), adParamInputOutput, dateTimeVal);
		pDB->CreateBinaryParameter(_T("@io_binaryVal"), adParamInputOutput, (BYTE*)binaryVal, 10);

		// for null value
		pDB->CreateParameter(_T("@Io_nullCharVal"), adParamInputOutput, NULL, 1);
		pDB->CreateNullParameter(_T("@io_nullVal"), adInteger, adParamInputOutput);
		pDB->CreateBinaryParameter(_T("@Io_nullBinaryVal"), adParamInputOutput, NULL, 1);


		pDB->SetQuery(_T("sample_procedure"));
		pDB->Execute();

		while (pDB->HasNext())
		{
			pDB->GetFieldValue(_T("bitVal"), bitField);
			pDB->GetFieldValue(_T("intVal"), intField);
			pDB->GetFieldValue(_T("charVal"), charField, MAX_STR_LEN);
			pDB->GetFieldValue(_T("datetimeVal"), dateTimeField);
			pDB->GetFieldValue(_T("binaryVal"), binaryValField, MAX_BIN_LEN, nSize);

			_tprintf(_T("query1 %d %d %s %s "), bitField, intField, charField, dateTimeField.Format(_T("%Y-%m-%d %H:%M:%S")));
			for (int i = 0; i < nSize; ++i)
				_tprintf(_T("%02X"), binaryValField[i]);
			_tprintf(_T("\n"));
		}

		pDB->NextRecordSet();

		while (pDB->HasNext())
		{
			pDB->GetFieldValue(_T("smallVal"), smallVal);
			pDB->GetFieldValue(_T("floatVal"), floatVal);
			pDB->GetFieldValue(_T("bigVal"), bigVal);

			_tprintf(_T("query2 %d %f %I64d\n"), smallVal, floatVal, bigVal);
		}

		pDB->GetParameter(_T("@io_bitVal"), bitVal);
		pDB->GetParameter(_T("@io_tinyVal"), tinyVal);
		pDB->GetParameter(_T("@io_smallVal"), smallVal);
		pDB->GetParameter(_T("@io_bigVal"), bigVal);
		pDB->GetParameter(_T("@io_floatval"), floatVal);
		pDB->GetParameter(_T("@io_smalldatetimeVal"), smallDateTimeVal);
		pDB->GetParameter(_T("@io_datetimeVal"), dateTimeVal);
		pDB->GetParameter(_T("@io_charVal"), charVal, MAX_STR_LEN);
		pDB->GetParameter(_T("@io_binaryVal"), binaryVal, MAX_BIN_LEN, nSize);

		_tprintf(_T("param out: %s\n"), charVal);
		_tprintf(_T("param out: %d\n"), bitVal);
		_tprintf(_T("param out: %d\n"), tinyVal);
		_tprintf(_T("param out: %d\n"), smallVal);
		_tprintf(_T("param out: %I64d\n"), bigVal);
		_tprintf(_T("param out: %s\n"), smallDateTimeVal.Format(_T("%Y-%m-%d %H:%M:%S")));
		_tprintf(_T("param out: %s\n"), dateTimeVal.Format(_T("%Y-%m-%d %H:%M:%S")));
		_tprintf(_T("param out: %.10lf\n"), floatVal);
		_tprintf(_T("param out: "));
		for (int j = 0; j < nSize; ++j)
			_tprintf(_T("%02X"), binaryValField[j]);
		_tprintf(_T("\n"));
	}
	catch (_com_error& e)
	{
		dump_com_error(e);
	}

	return 0;
}

int sample_raw_query()
{
	try
	{
		auto pDB = ADO::GetDB();
		if (!pDB)
			return 0;

		pDB->SetQuery(_T("Insert Into sample_table Values(1, 127, 20000, 12345678, 888888888888, 3.14, 'this is string!!', '20160308', '20160308', null, null, null, null)"));
		pDB->Execute(adCmdText);

		pDB = ADO::GetDB();
		if (!pDB)
			return 0;

		pDB->SetQuery(_T("select top 1 * from sample_table"));
		pDB->Execute(adCmdText);
		int intVal = 0;
		while (pDB->HasNext())
		{
			pDB->GetFieldValue(_T("intVal"), intVal);
			_tprintf(_T("query3 %d\n"), intVal);
		}
	}
	catch (_com_error& e)
	{
		dump_com_error(e);
	}


	return 0;
}

