#pragma once


#include <thread>
#include <list>
#include <mutex>
#include <atlcomtime.h>
#include <iostream>
#import "C:\Program Files\Common Files\System\ADO\msado15.dll" rename("EOF", "EndOfFile") no_namespace

using namespace std;

namespace std
{
#ifdef	_UNICODE
	typedef wstring tstring;
	static wostream& tcout = wcout;
	static wistream& tcin = wcin;
#else
	typedef string tstring;
	static ostream& tcout = cout;
	static istream& tcin = cin;
#endif
}

namespace ADO_NS
{

	struct CONFIG_IMPL;
	class CONFIG
	{
	public:
		friend class CADO;
		CONFIG(TCHAR* szIpPort, TCHAR* szDSN, TCHAR* szUserID, TCHAR* szPassword);
		CONFIG(CONFIG& r);
		~CONFIG();

		void SetInitCatalog(TCHAR* pString);
		void SetCommandTimeout(int nCommendTimeout);
		void SetConnectionTimeout(int nConnectionTimeout);
		void SetRetryConnection(bool bRetryConnection);

	private:
		tstring GetConnectionString();;
		CONFIG_IMPL* pImpl_;
	};

	struct CADO_IMPL;
	class CADO
	{
	public:
		CADO(CONFIG&);
		~CADO();

		void Open(CursorLocationEnum CursorLocation = adUseClient);
		void Close();
		void CommandReset();
		bool IsOpen();
		void SetQuery(IN TCHAR* tszQuery);;
		void Execute(CommandTypeEnum CommandType = adCmdStoredProc, ExecuteOptionEnum OptionType = adOptionUnspecified);
		
		int GetFieldCount();
		bool HasNext();
		void NextRecordSet();
		bool GetRetryConnection() const;

		void BeginTransaction();
		void CommitTransaction();
		void RollbackTransaction();
		
		template<typename T>
		void GetFieldValue(IN TCHAR* tszName, OUT T& Value);
		/**
		\remarks	문자형 필드값을 읽는다.
		*/
		void GetFieldValue(IN TCHAR*, OUT TCHAR*, IN unsigned int);
		/**
		\remarks	binary 필드값을 읽는다.
		*/
		void GetFieldValue(IN TCHAR*, OUT BYTE*, IN int, OUT int&);

		/**
		\remarks	정수/실수/날짜시간 타입의 파라메터 생성
		\par		null값의 파라메터 생성은 CreateNullParameter을 사용
		*/
		template<typename T>
		void CreateParameter(IN TCHAR* tszName, IN enum ParameterDirectionEnum Direction, IN T& rValue);

		/**
		\remarks	정수/실수/날짜시간 타입의 null값 파라메터 생성
		*/
		void CreateNullParameter(IN TCHAR*, IN enum DataTypeEnum, IN enum ParameterDirectionEnum);

		/**
		\remarks	문자열 타입 파라메터 생성, 길이 변수는 최소 0보다 커야 한다. null값 생성은 TCHAR*에 NULL값을 넘긴다.
		*/
		void CreateParameter(IN TCHAR*, IN enum ParameterDirectionEnum, IN TCHAR*, IN int);
		/**
		\remarks	binary 타입 파라메터 생성, 길이 변수는 최소 0보다 커야 한다. null값 생성은 BYTE*에 NULL값을 넘긴다.
		*/
		void CreateBinaryParameter(IN TCHAR*, IN enum ParameterDirectionEnum, IN BYTE*, IN int);

		/**
		\remarks	정수/실수/날짜시간 타입의 파라메터 값 읽기
		*/
		template<typename T>
		void GetParameter(TCHAR* tszName, OUT T& Value);

		/**
		\remarks	문자형 파라메터값을 읽는다.
		*/
		void GetParameter(IN TCHAR*, OUT TCHAR*, IN int);

		/**
		\remarks	바이너리형 파라메터값을 읽는다.
		*/
		void GetParameter(IN TCHAR*, OUT BYTE*, IN int, OUT int&);


	private:
		CADO_IMPL* pImpl;
	};



	class ADO
	{
	public:
		static void Initialize(CONFIG& cfg);
		static void Free();
		static CADO* GetDB();

	private:
		static int tls_index;
		static CONFIG* config;
		static std::mutex mt;
		static std::list<CADO*> ado_list;
	};
	
	static void dump_com_error(_com_error &e);
}


#include "ado_priv.h"
	


