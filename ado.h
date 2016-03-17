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
		\remarks	������ �ʵ尪�� �д´�.
		*/
		void GetFieldValue(IN TCHAR*, OUT TCHAR*, IN unsigned int);
		/**
		\remarks	binary �ʵ尪�� �д´�.
		*/
		void GetFieldValue(IN TCHAR*, OUT BYTE*, IN int, OUT int&);

		/**
		\remarks	����/�Ǽ�/��¥�ð� Ÿ���� �Ķ���� ����
		\par		null���� �Ķ���� ������ CreateNullParameter�� ���
		*/
		template<typename T>
		void CreateParameter(IN TCHAR* tszName, IN enum ParameterDirectionEnum Direction, IN T& rValue);

		/**
		\remarks	����/�Ǽ�/��¥�ð� Ÿ���� null�� �Ķ���� ����
		*/
		void CreateNullParameter(IN TCHAR*, IN enum DataTypeEnum, IN enum ParameterDirectionEnum);

		/**
		\remarks	���ڿ� Ÿ�� �Ķ���� ����, ���� ������ �ּ� 0���� Ŀ�� �Ѵ�. null�� ������ TCHAR*�� NULL���� �ѱ��.
		*/
		void CreateParameter(IN TCHAR*, IN enum ParameterDirectionEnum, IN TCHAR*, IN int);
		/**
		\remarks	binary Ÿ�� �Ķ���� ����, ���� ������ �ּ� 0���� Ŀ�� �Ѵ�. null�� ������ BYTE*�� NULL���� �ѱ��.
		*/
		void CreateBinaryParameter(IN TCHAR*, IN enum ParameterDirectionEnum, IN BYTE*, IN int);

		/**
		\remarks	����/�Ǽ�/��¥�ð� Ÿ���� �Ķ���� �� �б�
		*/
		template<typename T>
		void GetParameter(TCHAR* tszName, OUT T& Value);

		/**
		\remarks	������ �Ķ���Ͱ��� �д´�.
		*/
		void GetParameter(IN TCHAR*, OUT TCHAR*, IN int);

		/**
		\remarks	���̳ʸ��� �Ķ���Ͱ��� �д´�.
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
	


