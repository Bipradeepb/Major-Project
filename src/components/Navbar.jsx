import React from 'react';
import {
  AppBar,
  Toolbar,
  Typography,
  Button,
  Box,
  IconButton,
  Tooltip,
} from '@mui/material';
import LightModeIcon from '@mui/icons-material/LightMode';
import DarkModeIcon from '@mui/icons-material/DarkMode';
import { useTheme } from '@mui/material/styles';
import { useColorMode } from '../theme';

function NavBar() {
  const theme = useTheme();
  const colorMode = useColorMode();

  return (
    <AppBar
      position="sticky"
      elevation={0}
      sx={{
        bgcolor: 'background.default',
        color: 'text.primary',       // force text/icon color
      }}
    >
      <Toolbar
        sx={{
          mx: 'auto',
          marginLeft:'0',
          maxWidth:1200,
          justifyContent: 'space-between',
          width:'100%',
        }}
      >
        <Box sx={{ display: 'flex', alignItems: 'center',justifyContent:'space-evenly' }}>
          <Typography
            variant="h6"
            sx={{
              fontWeight: 'bold',
              color: 'hsl(210, 100%, 60%)',
              mr: 4,
            }}
          >
            HERMES
          </Typography>
          {['Home', 'Features', 'Contact'].map((text) => (
            <Button
              key={text}
              component="a"
              href={`#${text.toLowerCase()}`}
              sx={{
                textTransform: 'none',
                color: 'text.primary',
                mx: 1,
                fontWeight:700,
              }}
            >
              {text}
            </Button>
          ))}
        </Box>

        <Tooltip title="Toggle light/dark mode">
          <IconButton
            onClick={colorMode.toggleColorMode}
            sx={{ color: 'text.primary' }}    // ensure icon is visible
            >
            
               {theme.palette.mode === 'dark' ? (
                    <LightModeIcon sx={{ color: 'hsl(210, 100%, 60%)' }}/>
                    ) : (
                    <DarkModeIcon sx={{ color: 'hsl(210, 100%, 60%)' }}/>
                )}                    
          </IconButton>
        </Tooltip>
      </Toolbar>
    </AppBar>
  );
}

export default NavBar;
