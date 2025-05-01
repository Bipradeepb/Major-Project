import React, { useState, useMemo, createContext, useContext } from 'react';
import { createTheme, ThemeProvider } from '@mui/material/styles';

const ColorModeContext = createContext({ toggleColorMode: () => {} });
export function useColorMode() {
  return useContext(ColorModeContext);
}

export default function ToggleColorModeProvider({ children }) {
  const [mode, setMode] = useState('dark');
  const colorMode = useMemo(
    () => ({ toggleColorMode: () => setMode(prev => (prev === 'light' ? 'dark' : 'light')) }),
    []
  );
    const theme = useMemo(() => createTheme({
        palette: { mode },
        typography: {
        fontFamily: [
          '"IBM Plex Sans"',
          '-apple-system',
          'BlinkMacSystemFont',
          '"Segoe UI"',
          'Roboto',
          '"Helvetica Neue"',
          'Arial',
          'sans-serif',
          '"Apple Color Emoji"',
          '"Segoe UI Emoji"',
          '"Segoe UI Symbol"',
        ].join(','),
      },
    }), [mode]);

  return (
    <ColorModeContext.Provider value={colorMode}>
      <ThemeProvider theme={theme}>{children}</ThemeProvider>
    </ColorModeContext.Provider>
  );
}