import { defineConfig } from 'vite';
import tailwindcss from '@tailwindcss/vite';
import htmlMinify from 'vite-plugin-html-minify';
import { viteSingleFile } from "vite-plugin-singlefile"
import path from 'path';
import fs from 'fs';

export default defineConfig({
    build: {
        rollupOptions: {
            input: Object.fromEntries(
                fs.readdirSync('./in')
                    .filter(file => file.endsWith('.html'))
                    .map(file => [file.replace('.html', ''), path.resolve(__dirname, './in', file)])
            )
        }
    },
    plugins: [
        tailwindcss(),
        viteSingleFile({
            removeViteModuleLoader: true,
        }),
        htmlMinify(),
    ],
})
