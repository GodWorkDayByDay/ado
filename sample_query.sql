
CREATE TABLE [dbo].[sample_table](
	[bitVal] [bit] NULL,
	[tinyVal] [tinyint] NULL,
	[smallVal] [smallint] NULL,
	[intVal] [int] NULL,
	[bigVal] [bigint] NULL,
	[floatVal] [float] NULL,
	[charVal] [varchar](30) NULL,
	[smalldatetimeVal] [smalldatetime] NULL,
	[datetimeVal] [datetime] NULL,
	[binaryVal] [varbinary](50) NULL,
	[nullVal] [int] NULL,
	[nullCharVal] [varchar](30) NULL,
	[nullBinaryVal] [image] NULL
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]

GO

SET ANSI_PADDING OFF
GO


create  PROC [dbo].[sample_procedure]
    @io_bitVal		    BIT				OUTPUT,
    @io_tinyVal			TINYINT			OUTPUT,
    @io_smallVal	    SMALLINT		OUTPUT,
    @io_intVal          INT				OUTPUT,
    @io_bigVal		    BIGINT			OUTPUT,
    @io_floatVal		FLOAT			OUTPUT,
    @io_charVal         VARCHAR(30)		OUTPUT,
    @io_sDate		    SMALLDATETIME	OUTPUT,
    @io_date			DATETIME		OUTPUT,
    @io_binaryVal		VARBINARY(50)	OUTPUT,
	@io_nullVal			int				OUTPUT,
	@io_nullcharVal     VARCHAR(30)		OUTPUT,
    @io_nullBinaryVal	IMAGE			OUTPUT

AS       
DECLARE  
    @v_Flag             BIT
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON

    INSERT INTO dbo.sample_table values(
	@io_bitVal		 
	,@io_tinyVal		
	,@io_smallVal	 
	,@io_intVal       
	,@io_bigVal		 
	,@io_floatVal	
	,@io_charVal      
	,@io_sDate		 
	,@io_date		
	,@io_binaryVal	
	,@io_nullVal		
	,@io_nullcharVal  
	,@io_nullBinaryVal
	)
    
	select top 1 * from sample_table where binaryVal is not null
	select top 1 * from sample_table where binaryVal is not null

	set @io_intVal = 77777777
	SET @io_bigVal = 9223372036854775800
	SET @io_charVal = 'hello world!!!'
	SET @io_date = '20160307 23:23:11'
	SELECT top 1 @io_binaryVal = binaryVal from sample_table where binaryVal is not null
		
	return 1234
END

