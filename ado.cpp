#include "ado.h"

namespace ADO_NS
{
	struct CONFIG_IMPL
	{
		tstring m_tstrConnectingString_;
		tstring m_tstrInitialCatalog_;
		tstring m_tstrDataSource_;
		tstring m_tstrUserID_;
		tstring m_tstrPassword_;
		tstring m_tstrProvider_;
		tstring m_tstrDSN_;
		int m_nConnectionTimeout_;
		int m_nCommandTimeout_;
		bool m_bRetryConnection_;
	};


	CONFIG::CONFIG(TCHAR* szIpPort, TCHAR* szDSN, TCHAR* szUserID, TCHAR* szPassword)
		:pImpl_(new CONFIG_IMPL)
	{
		pImpl_->m_nConnectionTimeout_ = 0;
		pImpl_->m_nCommandTimeout_ = 0;
		pImpl_->m_bRetryConnection_ = false;

		if (szIpPort)
		{
			pImpl_->m_tstrDataSource_ = _T("Data Source=");
			pImpl_->m_tstrDataSource_ += szIpPort;
			pImpl_->m_tstrProvider_ = _T("SQLOLEDB.1");
		}

		if (szDSN)
		{
			pImpl_->m_tstrDSN_ = _T("DSN=");
			pImpl_->m_tstrDSN_ += szDSN;
		}

		pImpl_->m_tstrUserID_ = szUserID;
		pImpl_->m_tstrPassword_ = szPassword;
	}

	CONFIG::CONFIG(CONFIG& r)
		: pImpl_(new CONFIG_IMPL)
	{
		*pImpl_ = *r.pImpl_;
	}

	CONFIG::~CONFIG()
	{
		delete pImpl_;
	}

	void CONFIG::SetInitCatalog(TCHAR* pString)
	{
		pImpl_->m_tstrInitialCatalog_ = pString;
	}

	void CONFIG::SetCommandTimeout(int nCommendTimeout)
	{
		pImpl_->m_nCommandTimeout_ = nCommendTimeout;
	}

	void CONFIG::SetConnectionTimeout(int nConnectionTimeout)
	{
		pImpl_->m_nConnectionTimeout_ = nConnectionTimeout;
	}

	void CONFIG::SetRetryConnection(bool bRetryConnection)
	{
		pImpl_->m_bRetryConnection_ = bRetryConnection;
	}

	std::tstring CONFIG::GetConnectionString()
	{
		if (pImpl_->m_tstrDataSource_.empty())
			pImpl_->m_tstrConnectingString_ = pImpl_->m_tstrDSN_;
		else
			pImpl_->m_tstrConnectingString_ = pImpl_->m_tstrDataSource_;

		return pImpl_->m_tstrConnectingString_;
	}

	CADO::CADO(CONFIG& cfg)
		:pImpl(new CADO_IMPL(cfg))
	{
		if (FAILED(::CoInitialize(NULL))) {
			_tprintf(_T("::CoInitialize Fail!!"));
			return;
		}
		pImpl->m_pRecordset_ = nullptr;
		pImpl->m_pConnection_.CreateInstance(__uuidof(Connection));
		pImpl->m_pCommand_.CreateInstance(__uuidof(Command));
	}

	CADO::~CADO()
	{
		Close();
		delete pImpl;
	}

	void CADO::Open(CursorLocationEnum CursorLocation)
	{
		pImpl->m_pConnection_.Release();
		pImpl->m_pConnection_.CreateInstance(__uuidof(Connection));
		pImpl->m_pCommand_.Release();
		pImpl->m_pCommand_.CreateInstance(__uuidof(Command));

		CONFIG_IMPL* pConfigImpl = pImpl->config_.pImpl_;
		if (pConfigImpl->m_nConnectionTimeout_)
			pImpl->m_pConnection_->PutConnectionTimeout(pConfigImpl->m_nConnectionTimeout_);

		pImpl->m_pConnection_->CursorLocation = CursorLocation;

		if (!pConfigImpl->m_tstrProvider_.empty())		//ip立加老 版快 Provider 荤侩
			pImpl->m_pConnection_->put_Provider((_bstr_t)pConfigImpl->m_tstrProvider_.c_str());

		pImpl->m_pConnection_->Open((_bstr_t)pImpl->config_.GetConnectionString().c_str(), (_bstr_t)pConfigImpl->m_tstrUserID_.c_str(),
			(_bstr_t)pConfigImpl->m_tstrPassword_.c_str(), NULL);

		if (!pConfigImpl->m_tstrInitialCatalog_.empty())
			pImpl->m_pConnection_->DefaultDatabase = pConfigImpl->m_tstrInitialCatalog_.c_str();

		pImpl->m_pCommand_->ActiveConnection = pImpl->m_pConnection_;
	}

	bool CADO::IsOpen()
	{
		if (!pImpl->m_pConnection_ || pImpl->m_pConnection_->GetState() == adStateClosed)
			return false;

		return true;
	}

	void CADO::SetQuery(IN TCHAR* tszQuery)
	{
		pImpl->m_tstrQuery_ = tszQuery;
	}

	void CADO::BeginTransaction()
	{
		pImpl->m_pConnection_->BeginTrans();
	}

	void CADO::CommitTransaction()
	{
		pImpl->m_pConnection_->CommitTrans();
	}

	void CADO::RollbackTransaction()
	{
		pImpl->m_pConnection_->RollbackTrans();
	}

	void CADO::CommandReset()
	{
		pImpl->m_tstrQuery_.clear();

		pImpl->m_pCommand_.Release();
		pImpl->m_pCommand_.CreateInstance(__uuidof(Command));
		pImpl->m_pCommand_->ActiveConnection = pImpl->m_pConnection_;
	}

	int CADO::GetFieldCount()
	{
		if (!pImpl->m_pRecordset_ || !pImpl->m_pRecordset_->GetFields())
			return 0;

		return pImpl->m_pRecordset_->GetFields()->GetCount();;
	}

	bool CADO::HasNext()
	{
		if (!pImpl->m_pRecordset_)
			return false;

		if (!pImpl->is_first_record_)
		{
			pImpl->m_pRecordset_->MoveNext();
		}

		pImpl->is_first_record_ = false;
		return !pImpl->m_pRecordset_->GetEndOfFile();
	}

	void CADO::Execute(CommandTypeEnum CommandType /*= adCmdStoredProc*/, ExecuteOptionEnum OptionType /*= adOptionUnspecified*/)
	{
		pImpl->m_pCommand_->CommandType = CommandType;
		pImpl->m_pCommand_->CommandText = pImpl->m_tstrQuery_.c_str();

		CONFIG_IMPL* pConfigImpl = pImpl->config_.pImpl_;
		if (0 != pConfigImpl->m_nCommandTimeout_)
			pImpl->m_pCommand_->CommandTimeout = pConfigImpl->m_nCommandTimeout_;
		pImpl->m_pRecordset_ = pImpl->m_pCommand_->Execute(NULL, NULL, OptionType);
		pImpl->is_first_record_ = true;
	}

	void CADO::NextRecordSet()
	{
		if (!pImpl->m_pRecordset_)
			return;

		_variant_t variantRec;
		variantRec.intVal = 0;
		pImpl->m_pRecordset_ = pImpl->m_pRecordset_->NextRecordset((_variant_t*)&variantRec);
		pImpl->is_first_record_ = true;
	}

	void CADO::CreateNullParameter(IN TCHAR* tszName, IN enum DataTypeEnum Type, IN enum ParameterDirectionEnum Direction)
	{
		_ParameterPtr pParametor(pImpl->m_pCommand_->CreateParameter(tszName, Type, Direction, 0));
		pImpl->m_pCommand_->Parameters->Append(pParametor);
		_variant_t vNull;
		vNull.ChangeType(VT_NULL);
		pParametor->Value = vNull;
	}

	bool CADO::GetRetryConnection() const
	{
		return pImpl->config_.pImpl_->m_bRetryConnection_;
	}

	void CADO::Close()
	{
		try
		{
			if (pImpl->m_pConnection_->GetState() != adStateClosed)
				pImpl->m_pConnection_->Close();
		}
		catch (...)
		{

		}
	}

	void ADO::Initialize(CONFIG& cfg)
	{
		config = new CONFIG(cfg);

		if (!tls_index)
		{
			tls_index = ::TlsAlloc();
			if (tls_index == TLS_OUT_OF_INDEXES)
			{
				_tprintf(_T("%s error"), __FUNCTION__);
				return;
			}
		}
	}

	CADO* ADO::GetDB()
	{
		if (!tls_index || !config)
			return nullptr;

		CADO* p = (CADO*)TlsGetValue(tls_index);
		if (p)
		{
			if (p->IsOpen())
			{
				p->CommandReset();
			}
			else
			{
				if (p->GetRetryConnection())
				{
					p->Open();
				}
			}

			return nullptr;
		}
		else
		{
			CADO* p = new CADO(*config);
			p->Open();

			std::lock_guard<std::mutex> lock(mt);
			ado_list.push_back(p);

			TlsSetValue(tls_index, p);

			return p;
		}

		return nullptr;
	}

	void ADO::Free()
	{
		TlsFree(tls_index);
		tls_index = 0;

		std::lock_guard<std::mutex> lock(mt);
		for (auto it : ado_list)
			delete it;

		ado_list.clear();
	}

	int ADO::tls_index = 0;
	CONFIG* ADO::config = nullptr;
	std::mutex ADO::mt;
	std::list<CADO*> ADO::ado_list;
}

