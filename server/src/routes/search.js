// =============================================================================
// Switch App Store - Search Routes
// =============================================================================
// GET /api/search?q=query - Search games
// =============================================================================

const express = require('express');
const router = express.Router();
const { searchGames, games } = require('../data/games');

// =============================================================================
// GET /api/search
// Search games by query
// =============================================================================

router.get('/', (req, res) => {
    const { q, category, limit = 20, offset = 0 } = req.query;

    if (!q || q.length < 1) {
        return res.json({
            success: true,
            data: {
                query: '',
                results: [],
                total: 0
            }
        });
    }

    let results = searchGames(q);

    // Filter by category if specified
    if (category) {
        results = results.filter(g => g.category === category);
    }

    // Apply pagination
    const total = results.length;
    results = results.slice(parseInt(offset), parseInt(offset) + parseInt(limit));

    res.json({
        success: true,
        data: {
            query: q,
            results: results,
            total: total,
            limit: parseInt(limit),
            offset: parseInt(offset)
        }
    });
});

// =============================================================================
// GET /api/search/suggestions
// Get search suggestions (hot/trending keywords)
// =============================================================================

router.get('/suggestions', (req, res) => {
    // Return popular search terms
    const suggestions = [
        "RetroArch",
        "模拟器",
        "PPSSPP",
        "主题",
        "Goldleaf",
        "Tinfoil",
        "Doom",
        "2048"
    ];

    res.json({
        success: true,
        data: {
            hot: suggestions,
            trending: suggestions.slice(0, 4)
        }
    });
});

// =============================================================================
// GET /api/search/autocomplete
// Autocomplete for search input
// =============================================================================

router.get('/autocomplete', (req, res) => {
    const { q } = req.query;

    if (!q || q.length < 2) {
        return res.json({
            success: true,
            data: []
        });
    }

    const lowerQuery = q.toLowerCase();
    const matches = games
        .filter(g => g.name.toLowerCase().includes(lowerQuery))
        .slice(0, 8)
        .map(g => ({
            id: g.id,
            name: g.name,
            category: g.category,
            iconUrl: g.iconUrl
        }));

    res.json({
        success: true,
        data: matches
    });
});

module.exports = router;
