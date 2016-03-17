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

		if (!pConfigImpl->m_tstrProvider_.empty())		//ip접속일 경우 Provider 사용
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
	}

	void CADO::GetFieldValue(IN TCHAR* tszName, OUT TCHAR* pszValue, IN unsigned int nSize)
	{
		if (!pImpl->m_pRecordset_)
			return;

		_variant_t& vFieldValue = pImpl->m_pRecordset_->GetCollect(tszName);


		if (VT_NULL == vFieldValue.vt || VT_EMPTY == vFieldValue.vt) {
			assert(0);
			return;
		}
		else if (VT_BSTR != vFieldValue.vt) {
			assert(0);
			return;
		} if (nSize < _tcslen((TCHAR*)(_bstr_t(vFieldValue.bstrVal)))) {
			assert(0);
			return;
		}

		_tcscpy_s(pszValue, nSize, (TCHAR*)static_cast<_bstr_t>(vFieldValue.bstrVal));
	}

	void CADO::GetFieldValue(IN TCHAR* tszName, OUT BYTE* pbyBuffer, IN int inSize, OUT int& outSize)
	{
		if (!pImpl->m_pRecordset_)
			return;

		_variant_t& vFieldValue = pImpl->m_pRecordset_->GetCollect(tszName);

		if (VT_NULL == vFieldValue.vt)
		{
			assert(0);
			return;
		}
		else if ((VT_ARRAY | VT_UI1) != vFieldValue.vt) {
			assert(0);
			return;
		}

		FieldPtr pField = pImpl->m_pRecordset_->Fields->GetItem(tszName);

		if (inSize < pField->ActualSize || inSize > 8060)
		{
			assert(0);
			return ;
		}

		outSize = pField->ActualSize;

		BYTE * pData = NULL;
		SafeArrayAccessData(vFieldValue.parray, (void HUGEP* FAR*)&pData);
		memcpy(pbyBuffer, pData, pField->ActualSize);
		SafeArrayUnaccessData(vFieldValue.parray);
	}



	void CADO::CreateNullParameter(IN TCHAR* tszName, IN enum DataTypeEnum Type, IN enum ParameterDirectionEnum Direction)
	{
		_ParameterPtr pParametor(pImpl->m_pCommand_->CreateParameter(tszName, Type, Direction, 0));
		pImpl->m_pCommand_->Parameters->Append(pParametor);
		_variant_t vNull;
		vNull.ChangeType(VT_NULL);
		pParametor->Value = vNull;
	}

	void CADO::CreateParameter(IN TCHAR* tszName, IN enum ParameterDirectionEnum Direction, IN TCHAR* pValue, IN int nSize)
	{
		_ParameterPtr pParametor(pImpl->m_pCommand_->CreateParameter(tszName, adVarChar, Direction, nSize));
		pImpl->m_pCommand_->Parameters->Append(pParametor);

		if (NULL == pValue)
		{
			_variant_t vValue;
			vValue.vt = VT_NULL;
			pParametor->Value = vValue;
		}
		else
		{
			_variant_t vValue(pValue);
			pParametor->Value = vValue;
		}
	}



	void CADO::CreateBinaryParameter(IN TCHAR* tszName, IN enum ParameterDirectionEnum Direction, IN BYTE* pValue, IN int nSize)
	{
	
		_ParameterPtr pParametor(pImpl->m_pCommand_->CreateParameter(tszName, adVarBinary, Direction, nSize));
		pImpl->m_pCommand_->Parameters->Append(pParametor);

		_variant_t vBinary;
		SAFEARRAY FAR *pArray = NULL;
		SAFEARRAYBOUND rarrayBound[1];

		if (NULL == pValue)		//명시적 null이거나 값이 null이라면
		{
			vBinary.vt = VT_NULL;
			pParametor->Value = vBinary;
		}
		else
		{
			vBinary.vt = VT_ARRAY | VT_UI1;
			rarrayBound[0].lLbound = 0;
			rarrayBound[0].cElements = nSize;
			pArray = SafeArrayCreate(VT_UI1, 1, rarrayBound);

			for (long n = 0; n < nSize; ++n)
			{
				SafeArrayPutElement(pArray, &n, &(pValue[n]));
			}
			vBinary.parray = pArray;
			pParametor->AppendChunk(vBinary);
		}
	}

	void CADO::GetParameter(IN TCHAR* tszName, OUT TCHAR* pValue, IN int nSize)
	{
		_variant_t& vFieldValue = pImpl->m_pCommand_->Parameters->GetItem(tszName)->Value;

		if (VT_NULL == vFieldValue.vt ||
			VT_EMPTY == vFieldValue.vt) {
			assert(0);
			return;
		}
		else if (VT_BSTR != vFieldValue.vt) {
			assert(0);
			return;
		}

		if (nSize < (int)_tcslen((TCHAR*)(_bstr_t(vFieldValue.bstrVal)))) {
			assert(0);
			return;
		}

		_tcscpy_s(pValue, nSize, (TCHAR*)(_bstr_t)vFieldValue);
	}

	void CADO::GetParameter(IN TCHAR* tszName, OUT BYTE* pBuffer, IN int inSize, OUT int& outSize)
	{
		_variant_t& vFieldValue = pImpl->m_pCommand_->Parameters->GetItem(tszName)->Value;

		if (VT_NULL == vFieldValue.vt) {
			assert(0);
			return;
		}
		else if ((VT_ARRAY | VT_UI1) != vFieldValue.vt) {
			assert(0);
			return;
		}

		int ElementSize = vFieldValue.parray->rgsabound[0].cElements;

		if (ElementSize > inSize || inSize > 8060)
		{
			assert(0);
			return;
		}

		BYTE * pData = NULL;
		SafeArrayAccessData(vFieldValue.parray, (void HUGEP* FAR*)&pData);
		memcpy(pBuffer, pData, ElementSize);
		SafeArrayUnaccessData(vFieldValue.parray);
		outSize = vFieldValue.parray->rgsabound[0].cElements;
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

