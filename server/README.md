# Switch App Store Server

后端 API 服务器，为 Switch App Store 提供游戏数据服务。

## 快速开始

```bash
cd server
npm install
npm start
```

服务器将在 `http://localhost:3000` 启动。

## API 接口

### 健康检查
```
GET /api/health
```

### 目录
```
GET /api/catalog              # 获取完整目录
GET /api/catalog/categories   # 获取分类列表
GET /api/catalog/category/:id # 获取分类下的游戏
GET /api/catalog/game/:id     # 获取游戏详情
POST /api/catalog/download/:id # 记录下载
```

### 搜索
```
GET /api/search?q=query       # 搜索游戏
GET /api/search/suggestions   # 热门搜索
GET /api/search/autocomplete  # 自动完成
```

### 推荐
```
GET /api/featured             # 今日推荐
GET /api/featured/today       # 今日卡片
GET /api/featured/popular     # 热门游戏
GET /api/featured/new         # 新品游戏
```

## 部署

可使用以下方式部署：
- **Vercel**: `npx vercel`
- **Railway**: 连接 GitHub 仓库
- **Docker**: 使用 Dockerfile
