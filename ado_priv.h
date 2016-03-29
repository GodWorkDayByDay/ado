#pragma once

namespace ADO_NS
{
	struct CADO_IMPL
	{
		CADO_IMPL( CONFIG& cfg ) : config_( cfg ){}

		_ConnectionPtr m_pConnection_;
		_RecordsetPtr m_pRecordset_;
		_CommandPtr m_pCommand_;

		CONFIG config_;

		tstring m_tstrQuery_;
		bool is_first_record_;
	};

	template<typename T>
	void CADO::GetFieldValue( IN TCHAR* tszName, OUT T& Value )
	{
		if ( !pImpl->m_pRecordset_ )
			return;

		_variant_t& vFieldValue = pImpl->m_pRecordset_->GetCollect( tszName );

		switch ( vFieldValue.vt )
		{
			case VT_BOOL:		//bool
			case VT_I1:			//BYTE WORD
			case VT_I2:
			case VT_UI1:
			case VT_I4:			//DWORD
			case VT_DECIMAL:	//INT64
			case VT_R8:			//float double
			case VT_DATE:
			{
				Value = vFieldValue;
			}
			break;
			case VT_NULL:
			case VT_EMPTY:
			assert( 0 );
			return;
			default:
			assert( 0 );
			return;
		}
	}

	template<size_t N>
	void CADO::GetFieldValue( IN TCHAR* tszName, OUT TCHAR( &data )[N] )
	{
		if ( !pImpl->m_pRecordset_ )
			return;

		_variant_t& vFieldValue = pImpl->m_pRecordset_->GetCollect( tszName );

		switch ( vFieldValue.vt )
		{
			case VT_BSTR:
			{
				_tcscpy_s( data, N, ( TCHAR* )static_cast< _bstr_t >( vFieldValue.bstrVal ) );
			}
			break;
			default:
			assert( 0 );
			return;
		}
	}

	template<size_t N>
	void CADO::GetFieldValue( IN TCHAR* tszName, OUT BYTE( &data )[N], OUT int& outSize )
	{
		if ( !pImpl->m_pRecordset_ )
			return;

		_variant_t& vFieldValue = pImpl->m_pRecordset_->GetCollect( tszName );

		if ( VT_NULL == vFieldValue.vt )
		{
			assert( 0 );
			return;
		}
		else if ( ( VT_ARRAY | VT_UI1 ) != vFieldValue.vt ) {
			assert( 0 );
			return;
		}

		FieldPtr pField = pImpl->m_pRecordset_->Fields->GetItem( tszName );

		if ( N < pField->ActualSize || N > 8060 )
		{
			assert( 0 );
			return;
		}

		outSize = pField->ActualSize;

		BYTE * pData = NULL;
		SafeArrayAccessData( vFieldValue.parray, ( void HUGEP* FAR* )&pData );
		memcpy( data, pData, pField->ActualSize );
		SafeArrayUnaccessData( vFieldValue.parray );
	}


	template<typename T> struct DataType{};
	template<>	struct DataType<bool>
	{
		const static DataTypeEnum value{ adBoolean };
	};
	template<>	struct DataType<char>
	{
		const static DataTypeEnum value{ adTinyInt };
	};
	template<>	struct DataType<unsigned char>
	{
		const static DataTypeEnum value{ adTinyInt };
	};
	template<>	struct DataType<short>
	{
		const static DataTypeEnum value{ adSmallInt };
	};
	template<>	struct DataType<unsigned short>
	{
		const static DataTypeEnum value{ adSmallInt };
	};
	template<>	struct DataType<int>
	{
		const static DataTypeEnum value{ adInteger };
	};
	template<>	struct DataType<unsigned int>
	{
		const static DataTypeEnum value{ adInteger };
	};
	template<>	struct DataType<__int64>
	{
		const static DataTypeEnum value{ adBigInt };
	};
	template<>	struct DataType<unsigned __int64>
	{
		const static DataTypeEnum value{ adBigInt };
	};
	template<>	struct DataType<float>
	{
		const static DataTypeEnum value{ adDouble };
	};
	template<>	struct DataType<ATL::COleDateTime>
	{
		const static DataTypeEnum value{ adDBTimeStamp };
	};
	template<size_t N>	struct DataType<TCHAR[N]>
	{
		const static DataTypeEnum value{ adVarChar };
	};
	template<>	struct DataType<_variant_t>
	{};

	template<typename T = TCHAR*>
	inline TCHAR* getPointerValue( TCHAR* data )
	{
		return data;
	}

	template<typename T>
	inline T& getPointerValue( T& data )
	{
		return data;
	}

	template<typename T>
	inline size_t getSizeOf( T& data )
	{
		return sizeof( data );
	}

	template<typename T, size_t N>
	inline size_t getSizeOf( T( &)[N] )
	{
		return N;
	}

	template<typename T>
	void CADO::CreateParameter( IN TCHAR* tszName, IN enum ParameterDirectionEnum Direction, IN T& rValue )
	{
		_ParameterPtr pParametor = pImpl->m_pCommand_->CreateParameter( tszName, DataType<T>::value, Direction, getSizeOf( rValue ) );
		pImpl->m_pCommand_->Parameters->Append( pParametor );
		pParametor->Value = static_cast< _variant_t >( getPointerValue( rValue ) );
	}

	template<size_t N>
	void CADO::CreateParameter( IN TCHAR* tszName, IN enum ParameterDirectionEnum Direction, IN BYTE( &data )[N] )
	{

		_ParameterPtr pParametor( pImpl->m_pCommand_->CreateParameter( tszName, adVarBinary, Direction, N ) );
		pImpl->m_pCommand_->Parameters->Append( pParametor );

		_variant_t vBinary;
		SAFEARRAY FAR *pArray = NULL;
		SAFEARRAYBOUND rarrayBound[1];

		if ( NULL == data )		//명시적 null이거나 값이 null이라면
		{
			vBinary.vt = VT_NULL;
			pParametor->Value = vBinary;
		}
		else
		{
			vBinary.vt = VT_ARRAY | VT_UI1;
			rarrayBound[0].lLbound = 0;
			rarrayBound[0].cElements = N;
			pArray = SafeArrayCreate( VT_UI1, 1, rarrayBound );

			for ( long n = 0; n < N; ++n )
			{
				SafeArrayPutElement( pArray, &n, &( data[n] ) );
			}
			vBinary.parray = pArray;
			pParametor->AppendChunk( vBinary );
		}
	}

	template<typename T>
	void CADO::GetParameter( IN TCHAR* tszName, OUT T& Value )
	{
		_variant_t& vFieldValue = pImpl->m_pCommand_->Parameters->GetItem( tszName )->Value;

		switch ( vFieldValue.vt )
		{
			case VT_BOOL:	//bool
			case VT_I1:
			case VT_I2:		//BYTE WORD
			case VT_UI1:
			case VT_I4:		//DWORD
			case VT_DECIMAL: //INT64
			case VT_R8:	//float double
			case VT_DATE:
			Value = vFieldValue;
			break;
			case VT_NULL:
			case VT_EMPTY:
			assert( 0 );
			return;
			default:
			assert( 0 );
			return;
		}
	}

	template<size_t N>
	void CADO::GetParameter( IN TCHAR* tszName, OUT TCHAR( &data )[N] )
	{
		_variant_t& vFieldValue = pImpl->m_pCommand_->Parameters->GetItem( tszName )->Value;

		if ( VT_NULL == vFieldValue.vt ||
			VT_EMPTY == vFieldValue.vt ) {
			assert( 0 );
			return;
		}
		else if ( VT_BSTR != vFieldValue.vt ) {
			assert( 0 );
			return;
		}

		if ( N < ( int ) _tcslen( ( TCHAR* ) ( _bstr_t( vFieldValue.bstrVal ) ) ) ) {
			assert( 0 );
			return;
		}

		_tcscpy_s( data, N, ( TCHAR* ) ( _bstr_t ) vFieldValue );
	}

	template<size_t N>
	void CADO::GetParameter( IN TCHAR* tszName, OUT BYTE( &data )[N], OUT int& outSize )
	{
		_variant_t& vFieldValue = pImpl->m_pCommand_->Parameters->GetItem( tszName )->Value;

		if ( VT_NULL == vFieldValue.vt ) {
			assert( 0 );
			return;
		}
		else if ( ( VT_ARRAY | VT_UI1 ) != vFieldValue.vt ) {
			assert( 0 );
			return;
		}

		int ElementSize = vFieldValue.parray->rgsabound[0].cElements;

		if ( ElementSize > N || N > 8060 )
		{
			assert( 0 );
			return;
		}

		BYTE * pData = NULL;
		SafeArrayAccessData( vFieldValue.parray, ( void HUGEP* FAR* )&pData );
		memcpy( data, pData, ElementSize );
		SafeArrayUnaccessData( vFieldValue.parray );
		outSize = vFieldValue.parray->rgsabound[0].cElements;
	}


	void dump_com_error(_com_error &e)
	{
		_tprintf(_T("Code = %08lX   Code meaning = %s\n"), e.Error(), e.ErrorMessage());
		_tprintf(_T("Source = %s\n"), (_TCHAR*)e.Source());
		_tprintf(_T("Desc = %s\n"), (_TCHAR*)e.Description());
	}
}

