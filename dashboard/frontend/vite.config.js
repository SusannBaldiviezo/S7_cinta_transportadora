import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

export default defineConfig({
  plugins: [react()],
  // En desarrollo (npm run dev): las llamadas /api se redirigen al backend Python
  server: {
    port: 5173,
    proxy: {
      '/api': 'http://localhost:8080'
    }
  },
  // El build va a la carpeta static/ que sirve FastAPI
  build: {
    outDir: '../static',
    emptyOutDir: true,
  }
})
