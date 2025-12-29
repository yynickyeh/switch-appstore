// =============================================================================
// Switch App Store - Backend Server
// =============================================================================
// Express.js server providing REST API for game catalog
// =============================================================================

const express = require('express');
const cors = require('cors');
const morgan = require('morgan');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3000;

// =============================================================================
// Middleware
// =============================================================================

app.use(cors());
app.use(morgan('dev'));
app.use(express.json());

// Serve static files (game icons, screenshots)
app.use('/static', express.static(path.join(__dirname, '../public')));

// =============================================================================
// Routes
// =============================================================================

const catalogRoutes = require('./routes/catalog');
const searchRoutes = require('./routes/search');
const featuredRoutes = require('./routes/featured');

app.use('/api/catalog', catalogRoutes);
app.use('/api/search', searchRoutes);
app.use('/api/featured', featuredRoutes);

// =============================================================================
// Health check
// =============================================================================

app.get('/api/health', (req, res) => {
    res.json({
        status: 'ok',
        version: '1.0.0',
        timestamp: new Date().toISOString()
    });
});

// =============================================================================
// Error handling
// =============================================================================

app.use((err, req, res, next) => {
    console.error(err.stack);
    res.status(500).json({
        error: 'Internal server error',
        message: err.message
    });
});

// 404 handler
app.use((req, res) => {
    res.status(404).json({ error: 'Not found' });
});

// =============================================================================
// Start server
// =============================================================================

app.listen(PORT, () => {
    console.log(`🎮 Switch App Store Server running on port ${PORT}`);
    console.log(`📡 API: http://localhost:${PORT}/api`);
});
