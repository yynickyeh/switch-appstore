// =============================================================================
// Switch App Store - Catalog Routes
// =============================================================================
// GET /api/catalog - Get full catalog or by category
// =============================================================================

const express = require('express');
const router = express.Router();
const { games, categories, getGameById, getGamesByCategory } = require('../data/games');

// =============================================================================
// GET /api/catalog
// Returns full catalog with all games
// =============================================================================

router.get('/', (req, res) => {
    res.json({
        success: true,
        data: {
            games: games,
            categories: categories,
            total: games.length,
            lastUpdated: new Date().toISOString()
        }
    });
});

// =============================================================================
// GET /api/catalog/categories
// Returns all categories
// =============================================================================

router.get('/categories', (req, res) => {
    res.json({
        success: true,
        data: categories
    });
});

// =============================================================================
// GET /api/catalog/category/:categoryId
// Returns games in a specific category
// =============================================================================

router.get('/category/:categoryId', (req, res) => {
    const { categoryId } = req.params;
    const categoryGames = getGamesByCategory(categoryId);

    if (categoryGames.length === 0) {
        return res.status(404).json({
            success: false,
            error: 'Category not found or empty'
        });
    }

    res.json({
        success: true,
        data: {
            category: categoryId,
            games: categoryGames,
            total: categoryGames.length
        }
    });
});

// =============================================================================
// GET /api/catalog/game/:gameId
// Returns details for a specific game
// =============================================================================

router.get('/game/:gameId', (req, res) => {
    const { gameId } = req.params;
    const game = getGameById(gameId);

    if (!game) {
        return res.status(404).json({
            success: false,
            error: 'Game not found'
        });
    }

    res.json({
        success: true,
        data: game
    });
});

// =============================================================================
// POST /api/catalog/download/:gameId
// Track download (increment counter)
// =============================================================================

router.post('/download/:gameId', (req, res) => {
    const { gameId } = req.params;
    const game = getGameById(gameId);

    if (!game) {
        return res.status(404).json({
            success: false,
            error: 'Game not found'
        });
    }

    // Increment download count
    game.downloadCount++;

    res.json({
        success: true,
        data: {
            downloadUrl: game.downloadUrl,
            newDownloadCount: game.downloadCount
        }
    });
});

module.exports = router;
