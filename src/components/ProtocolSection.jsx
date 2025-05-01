import React from 'react';
import {Box,Divider,Container,Typography,Card,CardContent,useTheme} from '@mui/material';

function ProtocolSection({ title, description, screenshots, installCmd }) {
  const theme = useTheme();
  const isDark = theme.palette.mode === 'dark';

  return (
    <>
      <Box
        id={title.toLowerCase().replace(/\s+/g, '-')}
        sx={{
          py: 8,
          px: 2,
          backgroundRepeat: 'no-repeat',
          backgroundSize: 'cover',
          overflow: 'hidden',
          paddingTop:'10px',
          // backgroundImage: isDark
          // ? 'linear-gradient(var(--muidocs-palette-primaryDark-900) 0%, hsla(210, 100%, 23%, 0.2) 100%)'
          // : 'linear-gradient(to top, rgba(236, 246, 255, 0.8) 0%, rgba(255, 255, 255, 0) 90%)',
          backgroundImage: isDark
          ? 'linear-gradient(to top, rgba(0, 60, 115, 0.3) 0%, rgba(30, 32, 37, 0) 50%)'
          : 'linear-gradient(to top, rgba(236, 246, 255, 0.8) 0%, rgba(255, 255, 255, 0) 50%)',
          // boxShadow: isDark ? 6 : 3,
          // borderRadius: 4,
          my: 4,
          marginBottom:0
        }}
      >
        <Container maxWidth="md">
          <Typography
            variant="h4"
            gutterBottom
            sx={{ fontWeight: 'bold', color: 'text.primary' }}
          >
            {title}
          </Typography>
          <Typography
            variant="body1"
            paragraph
            sx={{ color: 'text.primary' }}
          >
            {description}
          </Typography>
          <Box sx={{ display: 'flex', gap: 2, overflowX: 'auto', mb: 4 }}>
            {screenshots.map((src) => (
              <Box
                component="img"
                key={src}
                src={src}
                alt={`${title} screenshot`}
                sx={{
                  width: '100%',
                  maxWidth: 300,
                  borderRadius: 1,
                  boxShadow: 3,
                }}
              />
            ))}
          </Box>
          <Card
            sx={{
              bgcolor: isDark ? '#1b263b' : '#f5f5f5',
              borderRadius: 2,
            }}
          >
            <CardContent>
              <Typography
                variant="subtitle1"
                sx={{
                  fontWeight: 'bold',
                  mb: 1,
                  color: isDark ? 'grey.100' : 'black',
                }}
              >
                How to Install?
              </Typography>
              <Box
                component="pre"
                sx={{
                  backgroundColor: isDark ? '#1e1e2f' : '#e0e0e0',
                  color: isDark ? 'grey.100' : 'black',
                  p: 2,
                  borderRadius: 1,
                  fontFamily: 'monospace',
                  overflowX: 'auto',
                }}
              >
                {installCmd}
              </Box>
            </CardContent>
          </Card>
        </Container>
      </Box>
      <Divider sx={{ my: 0 }} />
    </>  
  );
}

export default ProtocolSection;
