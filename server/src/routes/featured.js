// =============================================================================
// Switch App Store - Featured Routes
// =============================================================================
// GET /api/featured - Get featured and popular games
// =============================================================================

const express = require('express');
const router = express.Router();
const { getFeaturedGames, getPopularGames, games } = require('../data/games');

// =============================================================================
// GET /api/featured
// Get featured content for Today screen
// =============================================================================

router.get('/', (req, res) => {
    const featured = getFeaturedGames();
    const popular = getPopularGames(6);
    const recent = [...games]
        .sort((a, b) => new Date(b.releaseDate) - new Date(a.releaseDate))
        .slice(0, 6);

    res.json({
        success: true,
        data: {
            featured: featured,
            popular: popular,
            recent: recent,
            lastUpdated: new Date().toISOString()
        }
    });
});

// =============================================================================
// GET /api/featured/today
// Get "Today" card content
// =============================================================================

router.get('/today', (req, res) => {
    const todayCard = {
        type: "feature",
        title: "今日精选",
        subtitle: "本周最佳模拟器",
        game: getFeaturedGames()[0],
        backgroundColor: "#007AFF",
        textColor: "#FFFFFF"
    };

    res.json({
        success: true,
        data: todayCard
    });
});

// =============================================================================
// GET /api/featured/popular
// Get popular games
// =============================================================================

router.get('/popular', (req, res) => {
    const { limit = 10 } = req.query;
    const popular = getPopularGames(parseInt(limit));

    res.json({
        success: true,
        data: {
            games: popular,
            total: popular.length
        }
    });
});

// =============================================================================
// GET /api/featured/new
// Get new releases
// =============================================================================

router.get('/new', (req, res) => {
    const { limit = 10 } = req.query;
    const recent = [...games]
        .sort((a, b) => new Date(b.releaseDate) - new Date(a.releaseDate))
        .slice(0, parseInt(limit));

    res.json({
        success: true,
        data: {
            games: recent,
            total: recent.length
        }
    });
});

// =============================================================================
// GET /api/featured/categories/:categoryId
// Get featured games in a category
// =============================================================================

router.get('/categories/:categoryId', (req, res) => {
    const { categoryId } = req.params;
    const { limit = 6 } = req.query;

    const categoryGames = games
        .filter(g => g.category === categoryId)
        .sort((a, b) => b.downloadCount - a.downloadCount)
        .slice(0, parseInt(limit));

    res.json({
        success: true,
        data: {
            category: categoryId,
            games: categoryGames,
            total: categoryGames.length
        }
    });
});

module.exports = router;
