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

	template<typename T>
	struct DataT
	{
		typedef T& TT;
	};

	template<>
	struct DataT<TCHAR*>
	{
		typedef TCHAR* TT;
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
		void GetFieldValue(IN TCHAR* tszName, OUT T& data);
		template<size_t N>
		void GetFieldValue( IN TCHAR* tszName, OUT TCHAR(&data)[N] );
		/**
		\remarks	binary �ʵ尪�� �д´�.
		*/
		template<size_t N>
		void GetFieldValue(IN TCHAR* tszName, OUT BYTE(&data)[N], OUT int& outSize);

		/**
		\remarks	����/�Ǽ�/��¥�ð� Ÿ���� �Ķ���� ����
		\par		null���� �Ķ���� ������ CreateNullParameter�� ���
		*/
		template<typename T>
		void CreateParameter(IN TCHAR* tszName, IN enum ParameterDirectionEnum Direction, IN T& rValue);

		/**
		\remarks	����/�Ǽ�/��¥�ð� Ÿ���� null�� �Ķ���� ����
		*/
		void CreateNullParameter( IN TCHAR* tszName, IN enum DataTypeEnum Type, IN enum ParameterDirectionEnum Direction );

		/**
		\remarks	binary Ÿ�� �Ķ���� ����, ���� ������ �ּ� 0���� Ŀ�� �Ѵ�. null�� ������ BYTE*�� NULL���� �ѱ��.
		*/
		template<size_t N>
		void CreateParameter(IN TCHAR*, IN enum ParameterDirectionEnum, IN BYTE(&data)[N]);

		/**
		\remarks	����/�Ǽ�/��¥�ð� Ÿ���� �Ķ���� �� �б�
		*/
		template<typename T>
		void GetParameter(TCHAR* tszName, OUT T& Value);

		/**
		\remarks	������ �Ķ���Ͱ��� �д´�.
		*/
		template<size_t N>
		void GetParameter( IN TCHAR* tszName, OUT TCHAR( &data )[N] );

		/**
		\remarks	���̳ʸ��� �Ķ���Ͱ��� �д´�.
		*/
		template<size_t N>
		void GetParameter(IN TCHAR*, OUT BYTE(&data)[N], OUT int& outSize);


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
	


