/*  QEPlot.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The EPICS QT Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2009, 2010
 *
 *  Author:
 *    Glenn Jackson
 *  Contact details:
 *    glenn.jackson@synchrotron.org.au
 */

/*
  This class is a CA aware Plot widget and is based in part on the work of the Qwt project (http://qwt.sf.net).
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */

#include <qwt_legend.h>
#include <QEPlot.h>

/*
    Constructor with no initialisation
*/
QEPlot::QEPlot( QWidget *parent ) : QwtPlot( parent ), QEWidget( this ) {
    setup();
}

/*
    Constructor with known variable
*/
QEPlot::QEPlot( const QString &variableNameIn, QWidget *parent ) : QwtPlot( parent ), QEWidget( this ) {
    setup();
    setVariableName( variableNameIn, 0 );
}

/*
    Setup common to all constructors
*/
void QEPlot::setup() {
    // Set up data
    // This control used a single data source
    setNumVariables(QEPLOT_NUM_VARIABLES);

    // Set up default properties
    setAllowDrop( false );

    // Set the initial state
    isConnected = false;

    // General plot properties
    yMin = 0.0;
    yMax = 0.0;
    autoScale = true;
    axisEnableX = true;
    axisEnableY = true;

    //setLabelOrientation (Qt::Orientation)Qt::Vertical

    tickTimer = new QTimer(this);
    connect(tickTimer, SIGNAL(timeout()), this, SLOT(tickTimeout()));
    tickTimer->start( tickRate );

    // Waveform properties
    xStart = 0.0;
    xIncrement = 1.0;

    // Initially no curve or grid, and different trace colors
    for( int i = 0; i < QEPLOT_NUM_VARIABLES; i++ )
    {
        trace* tr = &traces[i];
        tr->curve = NULL;
        switch( i ) // Note, this assumes 4 traces, but won't break with more or less
        {
            case 0:  tr->color = Qt::black; break;
            case 1:  tr->color = Qt::red;   break;
            case 2:  tr->color = Qt::green; break;
            case 3:  tr->color = Qt::blue;  break;
            default: tr->color = Qt::black; break;
        }
        tr->style = QwtPlotCurve::Lines;
    }

    grid = NULL;
    gridEnableMajorX = false;
    gridEnableMajorY = false;
    gridEnableMinorX = false;
    gridEnableMinorY = false;
    gridMajorColor = Qt::black;
    gridMinorColor = Qt::gray;


    tickRate = 50;
    timeSpan = 59;

    // Use standard context menu
    setupContextMenu();

    // Use QwtPlot signals
    // !! move this functionality into QEWidget???
    // !! needs one for single variables and one for multiple variables, or just the multiple variable one for all
    // for each variable name property manager, set up an index to identify it when it signals and
    // set up a connection to recieve variable name property changes.
    // The variable name property manager class only delivers an updated variable name after the user has stopped typing
    for( int i = 0; i < QEPLOT_NUM_VARIABLES; i++ )
    {
        variableNamePropertyManagers[i].setVariableIndex( i );
        QObject::connect( &variableNamePropertyManagers[i], SIGNAL( newVariableNameProperty( QString, QString, unsigned int ) ),
                          this, SLOT( useNewVariableNameProperty( QString, QString, unsigned int ) ) );
    }
}

QEPlot::~QEPlot()
{
    if( tickTimer )
    {
        tickTimer->stop();
        delete tickTimer;
    }

    for( int i = 0; i < QEPLOT_NUM_VARIABLES; i++ )
    {
        trace* tr = &traces[i];
        if( tr->curve )
        {
            delete tr->curve;
            tr->curve = NULL;
        }
    }
    if( grid )
    {
        delete grid;
    }
}

/*
  Provides size hint in designer - in not a constraint
 */
QSize QEPlot::sizeHint() const
{
   return QSize (240, 100);
}

/*
    Implementation of QEWidget's virtual funtion to create the specific type of QCaObject required.
    For a strip chart a QCaObject that streams floating point data is required.
*/
qcaobject::QCaObject* QEPlot::createQcaItem( unsigned int variableIndex ) {

   // Create the item as a QEFloating
   return new QEFloating( getSubstitutedVariableName( variableIndex ), this, &floatingFormatting, variableIndex );
}

/*
    Start updating.
    Implementation of VariableNameManager's virtual funtion to establish a connection to a PV as the variable name has changed.
    This function may also be used to initiate updates when loaded as a plugin.
*/
void QEPlot::establishConnection( unsigned int variableIndex ) {

    // Create a connection.
    // If successfull, the QCaObject object that will supply data update signals will be returned
    qcaobject::QCaObject* qca = createConnection( variableIndex );

    // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots
    if(  qca ) {
        QObject::connect( qca,  SIGNAL( floatingArrayChanged( const QVector<double>&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                          this, SLOT( setPlotData( const QVector<double>&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
        QObject::connect( qca,  SIGNAL( floatingChanged( const double, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                          this, SLOT( setPlotData( const double, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
        QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo& , const unsigned int & ) ),
                          this, SLOT( connectionChanged( QCaConnectionInfo&, const unsigned int &  ) ) );
    }
}


/*
    Act on a connection change.
    Change how the strip chart looks and change the tool tip
    This is the slot used to recieve connection updates from a QCaObject based class.
 */
void QEPlot::connectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int & variableIndex)
{
    // Note the connected state
    isConnected = connectionInfo.isChannelConnected();

    // Display the connected state
    updateToolTipConnection( isConnected, variableIndex);
}

/*
    Update the plotted data with a new single value
    This is a slot used to recieve data updates from a QCaObject based class.
 */

void QEPlot::setPlotData( const double value, QCaAlarmInfo& alarmInfo, QCaDateTime& timestamp, const unsigned int& variableIndex ) {

    // Signal a database value change to any Link widgets
    emit dbValueChanged( value );

    // Select the curve information for this variable
    trace* tr = &traces[variableIndex];

    // Flag this trace is displaying a strip chart
    tr->waveform = false;

    // Add the new data point
    tr->timeStamps.append( timestamp );
    tr->ydata.append(value);
    tr->xdata.append( 0.0 ); // keep x and y arrays the same size
    regenerateTickXData( variableIndex );

    // Remove any old data
    QDateTime oldest = QDateTime::currentDateTime();
    oldest = oldest.addSecs( -(int)(timeSpan) );
    while( tr->timeStamps.count() > 1 )
    {
        if( tr->timeStamps[1] < oldest )
        {
            tr->timeStamps.remove(0);
            tr->xdata.remove(0);
            tr->ydata.remove(0);
        }
        else
        {
            break;
        }
    }

    // Fix the X for a strip chart
    setAxisScale( xBottom, -(double)timeSpan, 0.0 );

    // The data is now ready to plot
    setPlotDataCommon( variableIndex );
    setalarmInfoCommon( alarmInfo, variableIndex );
}

/*
    Update the plotted data with a new array of values
    This is a slot used to recieve data updates from a QCaObject based class.
 */
void QEPlot::setPlotData( const QVector<double>& values, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex ) {

    // Signal a database value change to any Link widgets
    emit dbValueChanged( values );

    // Select the curve information for this variable
    trace* tr = &traces[variableIndex];

    // Flag this trace is displaying a waveform
    tr->waveform = true;

    // Clear any previous data
    tr->xdata.clear();
    tr->ydata.clear();
    tr->timeStamps.clear();

    // If no increment was supplied, use 1 by default
    double inc;
    xIncrement == 0.0 ? inc = 1.0 : inc = xIncrement;

    for( int i = 0; i < values.count(); i++ )
    {
        tr->xdata.append( xStart + ((double)i * inc ) );
        tr->ydata.append( values[i] );
    }

    // Autoscale X for a waveform
    setAxisAutoScale( xBottom );

    // The data is now ready to plot
    setPlotDataCommon( variableIndex );
    setalarmInfoCommon( alarmInfo, variableIndex );
}

// Update the plot with new data.
// The new data may be due to a new value being added to the current values (stripchart)
// or the new data may be due to a new waveform
void QEPlot::setPlotDataCommon( const unsigned int variableIndex )
{
    trace* tr = &traces[variableIndex];

    // Create the curve if it does not exist
    if( !tr->curve )
    {
        tr->curve = new QwtPlotCurve( tr->legend );

        setCurveColor( tr->color, variableIndex );
        tr->curve->setRenderHint( QwtPlotItem::RenderAntialiased );
        tr->curve->setStyle( tr->style );
        tr->curve->attach(this);
    }

    // Set the curve data
#if QWT_VERSION >= 0x060000
    tr->curve->setSamples( tr->xdata, tr->ydata );
#else
    tr->curve->setData( tr->xdata, tr->ydata );
#endif
    // Update the plot
    replot();
}

void QEPlot::setalarmInfoCommon( QCaAlarmInfo& alarmInfo, const unsigned int variableIndex )
{
   // Invoke common alarm handling processing.
   // TODO: Aggregate all channel severities into a single alarm state.
   processAlarmInfo( alarmInfo, variableIndex );
}

/*
  For strip chart functionality
  Recalculate the x value as time goes by
 */
void QEPlot::regenerateTickXData( const unsigned int variableIndex )
{
    trace* tr = &traces[variableIndex];

    QCaDateTime now = QDateTime::currentDateTime();
    for( int i = 0; i < tr->xdata.count(); i++)
    {
        tr->xdata[i] = now.secondsTo (tr->timeStamps[i]);
    }
}

// Update the chart if it is a strip chart
void QEPlot::tickTimeout()
{
    for( int i = 0; i < QEPLOT_NUM_VARIABLES; i++ )
    {
        trace* tr = &traces[i];
        if( tr->curve && !tr->waveform )
        {
            regenerateTickXData( i );
            setPlotDataCommon( i );
        }
    }
}

// Update the color of the trace
void QEPlot::setCurveColor( const QColor color, const unsigned int variableIndex )
{
    trace* tr = &traces[variableIndex];
    if( tr->curve )
    {
        tr->curve->setPen( color );
    }
}

//==============================================================================
// Drag and Drop
void QEPlot::setDrop( QVariant drop )
{
    QStringList PVs = drop.toString().split( ' ' );
    for( int i = 0; i < PVs.size() && i < QEPLOT_NUM_VARIABLES; i++ )
    {
        setVariableName( PVs[i], i );
        establishConnection( i );
    }
}

QVariant QEPlot::getDrop()
{
    if( isDraggingVariable() )
        return QVariant( copyVariable() );
    else
        return copyData();
}

//==============================================================================
// Copy / Paste
QString QEPlot::copyVariable()
{
    QString text;
    for( int i = 0; i < QEPLOT_NUM_VARIABLES; i++ )
    {
        QString pv = getSubstitutedVariableName(i);
        if( !pv.isEmpty() )
        {
            if( !text.isEmpty() )
                text.append( " " );
            text.append( pv );
        }
    }

    return text;
//    return getSubstitutedVariableName(0);
}

QVariant QEPlot::copyData()
{
    QString text;
    for( int i = 0; i < QEPLOT_NUM_VARIABLES; i++ )
    {
        trace* tr = &traces[i];
        // Use i + 1 (as opposed to just i) as variable propety names are 1 to 4, not 0 to 3.
        text.append( QString( "\n%1\nx\ty\n" ).arg( tr->legend.isEmpty()?QString( "Variable %1" ).arg( i + 1 ):tr->legend ) );
        for( int j = 0; j < tr->xdata.count(); j++ )
        {
            text.append( QString( "%1\t%2\n" ).arg( tr->xdata[j] ).arg( tr->ydata[j]) );
        }
    }

    return QVariant( text );
}

void QEPlot::paste( QVariant v )
{
    if( getAllowDrop() )
    {
        setDrop( v );
    }
}

//==============================================================================
// Property convenience functions

// Access functions for YMin
void QEPlot::setYMin( double yMinIn )
{
    yMin = yMinIn;
    if( !autoScale )
    {
        setAxisScale( yLeft, yMin, yMax );
    }
}
double QEPlot::getYMin()
{
    return yMin;
}

// Access functions for yMax
void QEPlot::setYMax( double yMaxIn )
{
    yMax = yMaxIn;
    if( !autoScale )
    {
        setAxisScale( yLeft, yMin, yMax );
    }
}
double QEPlot::getYMax()
{
    return yMax;
}

// Access functions for autoScale
void QEPlot::setAutoScale( bool autoScaleIn )
{
    autoScale = autoScaleIn;
    
    // Set auto scale if requested, or if manual scale values are invalid
    if( autoScale || yMin == yMax )
    {
        setAxisAutoScale( yLeft );
    }
    else
    {
        setAxisScale( yLeft, yMin, yMax );
    }
}
bool QEPlot::getAutoScale()
{
    return autoScale;
}

// Access functions for X axis visibility
void QEPlot::setAxisEnableX( bool axisEnableXIn )
{
    axisEnableX = axisEnableXIn;
    enableAxis( xBottom, axisEnableX );
}

bool QEPlot::getAxisEnableX()
{
    return axisEnableX;
}

// Access functions for Y axis visibility
void QEPlot::setAxisEnableY( bool axisEnableYIn )
{
    axisEnableY = axisEnableYIn;
    enableAxis( yLeft, axisEnableY );
}

bool QEPlot::getAxisEnableY()
{
    return axisEnableY;
}

// Access functions for grid
void QEPlot::setGridEnableMajorX( bool gridEnableMajorXIn )
{
    gridEnableMajorX = gridEnableMajorXIn;
    setGridEnable();
}
void QEPlot::setGridEnableMajorY( bool gridEnableMajorYIn )
{
    gridEnableMajorY = gridEnableMajorYIn;
    setGridEnable();
}
void QEPlot::setGridEnableMinorX( bool gridEnableMinorXIn )
{
    gridEnableMinorX = gridEnableMinorXIn;
    setGridEnable();
}
void QEPlot::setGridEnableMinorY( bool gridEnableMinorYIn )
{
    gridEnableMinorY = gridEnableMinorYIn;
    setGridEnable();
}
void QEPlot::setGridEnable()
{
    // If any grid is required, create a grid and set it up
    // Note, Qwt will ignore minor enable if major is not enabled
    if( gridEnableMajorX || gridEnableMajorY || gridEnableMinorX || gridEnableMinorY )
    {
        if( !grid )
        {
            grid = new QwtPlotGrid;
#if QWT_VERSION >= 0x060100
            grid->setMajorPen(QPen(gridMajorColor, 0, Qt::DotLine));
            grid->setMinorPen(QPen(gridMinorColor, 0 , Qt::DotLine));
#else
            grid->setMajPen(QPen(gridMajorColor, 0, Qt::DotLine));
            grid->setMinPen(QPen(gridMinorColor, 0 , Qt::DotLine));
#endif
            grid->attach( this );
        }
        grid->enableX(gridEnableMajorX);
        grid->enableY(gridEnableMajorY);
        grid->enableXMin(gridEnableMinorX);
        grid->enableYMin(gridEnableMinorY);
    }
    // No grid required, get rid of any grid
    else
    {
        if( grid )
        {
            grid->detach();
            delete grid;
            grid = NULL;
        }
    }
}
bool QEPlot::getGridEnableMajorX()
{
    return gridEnableMajorX;
}
bool QEPlot::getGridEnableMajorY()
{
    return gridEnableMajorY;
}
bool QEPlot::getGridEnableMinorX()
{
    return gridEnableMinorX;
}
bool QEPlot::getGridEnableMinorY()
{
    return gridEnableMinorY;
}


// Access functions for gridColor
void QEPlot::setGridMajorColor( QColor gridMajorColorIn )
{
    gridMajorColor = gridMajorColorIn;
    if( grid )
    {
#if QWT_VERSION >= 0x060100
        grid->setMajorPen(QPen(gridMajorColor, 0, Qt::DotLine));
#else
        grid->setMajPen(QPen(gridMajorColor, 0, Qt::DotLine));
#endif
    }
}
void QEPlot::setGridMinorColor( QColor gridMinorColorIn )
{
    gridMinorColor = gridMinorColorIn;
    if( grid )
    {
#if QWT_VERSION >= 0x060100
        grid->setMinorPen(QPen(gridMinorColor, 0 , Qt::DotLine));
#else
        grid->setMinPen(QPen(gridMinorColor, 0 , Qt::DotLine));
#endif
    }
}
QColor QEPlot::getGridMajorColor()
{
    return gridMajorColor;
}
QColor QEPlot::getGridMinorColor()
{
    return gridMinorColor;
}


// Access functions for title
// No QEPlot::setTitle() needed. Uses QwtPlot::setTitle()
QString QEPlot::getTitle()
{
    return title().text();
}

// Access functions for backgroundColor
void    QEPlot::setBackgroundColor( QColor backgroundColor )
{
#if QWT_VERSION >= 0x060000
    QBrush brush = canvasBackground();
    brush.setColor( backgroundColor );
    setCanvasBackground( brush );
#else
    setCanvasBackground( backgroundColor );
#endif
}
QColor QEPlot::getBackgroundColor()
{
#if QWT_VERSION >= 0x060000
    return canvasBackground().color();
#else
    return canvasBackground();
#endif
}

// Access functions for traceStyle
void QEPlot::setTraceStyle( QwtPlotCurve::CurveStyle traceStyle, const unsigned int variableIndex ){
    trace* tr = &traces[variableIndex];
    tr->style = traceStyle;
    if( tr->curve )
    {
        tr->curve->setStyle( tr->style );
    }
}

QwtPlotCurve::CurveStyle QEPlot::getTraceStyle( const unsigned int variableIndex )
{
    return traces[variableIndex].style;
}

// Access functions for traceColor
void QEPlot::setTraceColor( QColor traceColor, const unsigned int variableIndex ){
    traces[variableIndex].color = traceColor;
    setCurveColor( traceColor, variableIndex );
}
void QEPlot::setTraceColor1( QColor traceColor ){ setTraceColor( traceColor, 0 ); }
void QEPlot::setTraceColor2( QColor traceColor ){ setTraceColor( traceColor, 1 ); }
void QEPlot::setTraceColor3( QColor traceColor ){ setTraceColor( traceColor, 2 ); }
void QEPlot::setTraceColor4( QColor traceColor ){ setTraceColor( traceColor, 3 ); }

QColor QEPlot::getTraceColor( const unsigned int variableIndex )
{
    return traces[variableIndex].color;
}
QColor QEPlot::getTraceColor1(){ return getTraceColor( 0 ); }
QColor QEPlot::getTraceColor2(){ return getTraceColor( 1 ); }
QColor QEPlot::getTraceColor3(){ return getTraceColor( 2 ); }
QColor QEPlot::getTraceColor4(){ return getTraceColor( 3 ); }

// Access functions for traceLegend
void QEPlot::setTraceLegend( QString traceLegend, const unsigned int variableIndex ){

    trace* tr = &traces[variableIndex];

    tr->legend = traceLegend;
    if( traceLegend.count() )
    {
        insertLegend( new QwtLegend(), QwtPlot::RightLegend );
    }
    else
    {
        insertLegend( NULL, QwtPlot::RightLegend );
    }

    if( tr->curve )
    {
        tr->curve->setTitle( traceLegend );
    }
}
void QEPlot::setTraceLegend1( QString traceLegend ){ setTraceLegend( traceLegend, 0 ); }
void QEPlot::setTraceLegend2( QString traceLegend ){ setTraceLegend( traceLegend, 1 ); }
void QEPlot::setTraceLegend3( QString traceLegend ){ setTraceLegend( traceLegend, 2 ); }
void QEPlot::setTraceLegend4( QString traceLegend ){ setTraceLegend( traceLegend, 3 ); }

QString QEPlot::getTraceLegend( const unsigned int variableIndex )
{
    return traces[variableIndex].legend;
}
QString QEPlot::getTraceLegend1(){ return getTraceLegend( 0 ); }
QString QEPlot::getTraceLegend2(){ return getTraceLegend( 1 ); }
QString QEPlot::getTraceLegend3(){ return getTraceLegend( 2 ); }
QString QEPlot::getTraceLegend4(){ return getTraceLegend( 3 ); }

// Access functions for xUnit
void    QEPlot::setXUnit( QString xUnit )
{
    setAxisTitle(xBottom, xUnit);
}
QString QEPlot::getXUnit()
{
    return axisTitle( xBottom ).text();
}

// Access functions for yUnit
void    QEPlot::setYUnit( QString yUnit )
{
    setAxisTitle( yLeft, yUnit );
}
QString QEPlot::getYUnit()
{
    return axisTitle( yLeft ).text();
}

// Access functions for xStart
void QEPlot::setXStart( double xStartIn )
{
    xStart = xStartIn;
}
double QEPlot::getXStart()
{
    return xStart;
}

// Access functions for xIncrement
void QEPlot::setXIncrement( double xIncrementIn )
{
    xIncrement = xIncrementIn;
}
double QEPlot::getXIncrement()
{
    return xIncrement;
}

// Access functions for timeSpan
void QEPlot::setTimeSpan( unsigned int timeSpanIn )
{
    timeSpan = timeSpanIn;
}
unsigned int QEPlot::getTimeSpan()
{
    return timeSpan;
}

// Access functions for tickRate
void QEPlot::setTickRate( unsigned int tickRateIn )
{
    tickRate = tickRateIn;
    if( tickTimer )
    {
        tickTimer->stop();
        tickTimer->start( tickRate );
    }
}
unsigned int QEPlot::getTickRate()
{
    return tickRate;
}

// end
